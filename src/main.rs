use rand;

use std::sync::mpsc::{Sender, Receiver};
use std::sync::mpsc;
use std::thread;
use std::clone::{Clone};
use std::collections::{HashMap};
use std::convert::TryFrom;

pub trait GroupBy<A> {
    fn group_by<F, K>(&self, f: F) -> HashMap<K, Vec<&A>>
    where F : Fn(&A) -> K,
          K: std::hash::Hash + std::cmp::Eq + Copy;
}

impl <A> GroupBy<A> for Vec<A> {
    fn group_by<F, K>(&self, f: F) -> HashMap<K, Vec<&A>>
    where F : Fn(&A) -> K,
          K: std::hash::Hash + std::cmp::Eq + Copy
    {
        let mut m = HashMap::new();
        for i in self {
            let k = f(&i);
            if ! m.contains_key(&k) {
                m.insert(k, Vec::new());
            }
            m.get_mut(&k).unwrap().push(i);
        };
        m
    }
}

#[derive(Clone, Debug)]
enum Msg{
    Return,
    Work {region : u32},
    GoldFound {amount: u32, miner: usize},
    RoundResult {winners: Vec<usize>, losers: Vec<usize> },
    Transfer {amount: u32, sender:usize, receiver: usize},
    Exit,
}

type Broadband = HashMap<usize, Sender<Msg>>;

trait Broadcast {
    fn cast(&self, src: usize, msg: Msg);
}

impl Broadcast for Broadband {
    fn cast(&self, src: usize, msg: Msg) {
        for (k, v) in self {
            if *k != src {
                v.send(msg.clone()).unwrap();
            }
        }
    }
}

fn miner_work(me: usize, rx: Receiver<Msg>, mut everybody: Broadband)  {
    let mut total :u32 = 0;
    println!("{}: Journal start", me);
    while let Ok(msg) = rx.recv() {
        match msg {
            Msg::Return => {
                let amount = rand::random::<u32>() % 10;
                total += amount;
                everybody.cast(me, Msg::GoldFound{amount, miner: me});
                println!("{}: Return with {} gold found ", me, amount)
            },
            Msg::Work {region} => {
                println!("{}: Working region {}", me, region)
            },
            Msg::GoldFound {amount, miner}  => {
                println!("{}: I hear miner {} found {}", me, miner, amount)
            },
            Msg::RoundResult {winners, losers} => {
                if losers.contains(&me) {
                    let transfer_amount = total / u32::try_from(winners.len()).unwrap();
                    for i in winners {
                        println!("{}: Transfering {} to {}", me, transfer_amount, i);
                        everybody[&i].send(Msg::Transfer {amount: transfer_amount, sender: me, receiver: i}).unwrap();
                    }
                    println!("{}: I'm fired :( . Bye... ", me);
                    break;
                } else {
                    for l in losers {
                        everybody.remove(&l);
                    }
                }
            },
            Msg::Transfer {amount, sender , receiver} => {
                if receiver == me {
                    println!("{}: Received {} from {}", me, amount, sender);
                    total += amount;
                }
            },
            Msg::Exit => {
                println!("{}: Journal Done. Collected {} gold total", me, total);
                break
            }
        }
    }
}

fn lead_round_results(round_data: &Vec<(u32, usize)>) -> (Vec<usize>, Vec<usize>) {
    let mut miners_by_amount : Vec<(u32, Vec<usize>)> = round_data
        .group_by(|a| a.0)
        .into_iter()
        .map(|p| (p.0, p.1.iter().map(|x|x.1).collect()))
        .collect();
    miners_by_amount.sort_by(|a,b| b.0.cmp(&a.0));
    let winners = miners_by_amount
        .first()
        .map(|p| p.1.clone())
        .unwrap_or(vec![]);
    let losers = miners_by_amount
        .last()
        .map(|p| p.1.clone())
        .unwrap_or(vec![]);
    (winners, losers)
}

fn lead_collect(rx: &Receiver<Msg>, active_miners: usize) -> Vec<(u32,usize)> {
    let mut round_data = Vec::new();
    for _i in 0..active_miners {
        match rx.recv() {
            Ok(Msg::GoldFound {amount, miner})  => {
                round_data.push((amount, miner));
                println!("{}: Recording {} gold found by {}", 0, amount, miner)
            },
            _ => {
                println!("Err");
                break;
            }
        }
    }
    round_data
}

fn lead_work(me: usize, rx: Receiver<Msg>, mut everybody: Broadband, regions: u32) {
    for r in 0..regions {
        let active_miners = everybody.len() - 1;
        if active_miners <= 1 {
            break;
        }
        everybody.cast(0, Msg::Work{region : r});
        everybody.cast(0, Msg::Return);
        let round_data = lead_collect(&rx, active_miners);
        let (winners, losers) = lead_round_results(&round_data);
        if losers.len() == 1 {
            let loser = losers[0];
            println!("{}: Region {} Winners {:?} Losers {}", me, r, winners, loser);
            everybody.cast(0, Msg::RoundResult { winners : winners, losers : losers});
            everybody.remove(&loser);
        }
    }
    println!("lead: exit!");
    everybody.cast(0, Msg::Exit);

}

fn main() {
    const R :u32 = 10;
    const M :usize = 10;

    // Initialization
    let mut txs = HashMap::new();
    let mut rxs = HashMap::new();
    for i in 0..M+1 {
        let (tx, rx) = mpsc::channel();
        txs.insert(i, tx);
        rxs.insert(i, rx);
    };

    // Execution
    let mut ts = Vec::new();
    for i in 1..M+1 {
        let rx = rxs.remove(&i).unwrap();
        let everybody : Broadband = txs
            .iter()
            .map(|a| (*a.0, a.1.clone()))
            .collect();
        let m = thread::spawn(move || {
            miner_work(i, rx, everybody);
        });
        ts.push(m);
    };

    let everybody = txs;
    lead_work(0, rxs.remove(&0).unwrap(), everybody, R);
    for t in ts {
        t.join().unwrap();
    }
    println!("main: done!");
}
