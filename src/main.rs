use rand::prelude::*;

use std::sync::Arc;
use std::sync::mpsc::{Sender, Receiver};
use std::sync::mpsc;
use std::thread;
use std::clone::{Clone};
use std::collections::{HashMap};

#[derive(Copy, Clone, Debug)]
enum Msg {
    Return,
    Work {region : u32},
    GoldFound {amount: u32, miner: usize},
    RoundResult {winner: usize, loser: usize},
    Transfer {amount: u32, sender:usize, receiver: usize},
    Exit,
}

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

fn broadcast<T:Clone>(txs: &Vec<Sender<T>>, src: usize, msg: T) {
    for i in 0..txs.len() {
        if i != src {
            txs[i].send(msg.clone());
        }
    }
}

fn miner_work(me: usize, rx: Receiver<Msg>, everybody: Vec<Sender<Msg>>)  {
    let mut total = 0;
    while let Ok(msg) = rx.recv() {
        match msg {
            Msg::Return => {
                let amount = rand::random::<u32>() % 10;
                broadcast(&everybody, me, Msg::GoldFound{amount, miner: me});
                println!("{}: Return", me)
            },
            Msg::Work {region} => {
                println!("{}: Work {}", me, region)
            },
            Msg::GoldFound {amount, miner}  => {
                println!("{}: Dont care", me)
            },
            Msg::RoundResult {winner, loser} => {
                if loser == me {
                    println!("{}: Im fired; Transfer {} {} {}", me ,total, me, winner);
                    break;
                } else {

                    println!("{}: Dont care", me)
                }
            },
            Msg::Transfer {amount, sender , receiver} => {
                if receiver == me {
                    total += amount;
                    println!("{}: Dont care", me)
                } else {
                    println!("{}: Dont care", me)
                }
            },
            Msg::Exit => {
                println!("{}: Exit", me);
                break
            }
        }
    }
}

fn round_results(round_data: &Vec<(u32, usize)>) -> (Vec<usize>, Vec<usize>) {
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

fn lead_work(me: usize, rx: Receiver<Msg>, everybody: Vec<Sender<Msg>>, regions: u32) {
    let activeMiners= everybody.len() - 1;
    for r in 0..regions {
        broadcast(&everybody, 0, Msg::Work{region : r});
        broadcast(&everybody, 0, Msg::Return);
        let mut round_data = Vec::new();
        for _i in 0..activeMiners {
            match rx.recv() {
                Ok(Msg::GoldFound {amount, miner})  => {
                    round_data.push((amount, miner));
                    println!("{}: {} {}", 0, amount, miner)
                },
                Ok(a) => {
                    println!("{}: {:?}", 0, a)
                },
                Err(a) => {
                    println!("Err {}", a);
                    break;
                }
            }
        }

        let (winners, losers) = round_results(&round_data);
        if losers.len() == 1 {
            // broadcast(&everybody, 0, Msg::RoundResult { winner : winners.first(), loser : losers.first()});
        }


    }
    println!("lead: exit!");
    broadcast(&everybody, 0, Msg::Exit);

}

fn main() {
    const R :u32 = 2;
    const M :usize = 10;

    // Initialization
    let mut txs = Vec::new();
    let mut rxs = Vec::new();
    let (lead_tx, lead_rx) = mpsc::channel();
    txs.push(lead_tx);

    for i in 0..M {
        let (tx, rx) = mpsc::channel();
        txs.push(tx);
        rxs.push(rx);
    };

    // Execution
    let mut ts = Vec::new();
    for i in (1..M+1).rev() {
        let rx = rxs.pop().unwrap();
        let everybody : Vec<Sender<Msg>>= txs.iter().map(|t| t.clone()).collect();
        let m = thread::spawn(move || {
            println!("{}: start!", i);
            miner_work(i, rx, everybody);
            println!("{}: finish!", i);
        });
        ts.push(m);
    };

    let everybody = txs;
    lead_work(0, lead_rx, everybody, R);
    for t in ts {
        t.join();
    }
    println!("lead: done!");
}
