use rand::prelude::*;

use std::sync::Arc;
use std::sync::mpsc::{Sender, Receiver};
use std::sync::mpsc;
use std::thread;
use std::clone::{Clone};

#[derive(Copy, Clone, Debug)]
enum Msg {
    Return,
    Work {region : u32},
    GoldFound {amount: u32, miner: usize},
    RoundResult {winner: usize, loser: usize},
    Transfer {amount: u32, sender:usize, receiver: usize},
    Exit,
}

fn broadcast<T:Clone>(txs: &Vec<Sender<T>>, src: usize, msg: T) {
    for i in 0..txs.len() {
        if i != src {
            txs[i].send(msg.clone());
        }
    }
}

fn miner_work(me: usize, rx: Receiver<Msg>, everybody: &Vec<Sender<Msg>>)  {
    let mut total = 0;
    while let Ok(msg) = rx.recv() {
        match msg {
            Msg::Return => {
                let amount = rand::random::<u32>();
                broadcast(everybody, me, Msg::GoldFound{amount, miner: me});
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
                    println!("{}: Im fired; Transfer {} {} {}", me ,total, me, winner)
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
            miner_work(i, rx, &everybody);
            println!("{}: finish!", i);
        });
        ts.push(m);
    };

    let everybody = txs;
    let activeMiners= M;
    for r in 0..R {
        broadcast(&everybody, 0, Msg::Work{region : r});
        broadcast(&everybody, 0, Msg::Return);
        for _i in 0..activeMiners {
            match lead_rx.recv() {
                Ok(Msg::GoldFound {amount, miner})  => {
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
    }
    println!("lead: exit!");
    broadcast(&everybody, 0, Msg::Exit);
    for t in ts {
        t.join();
    }
    println!("lead: done!");
}
