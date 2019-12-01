
use crate::messages::*;
use std::convert::TryFrom;
use std::sync::mpsc::{Receiver};
use crate::logger::log;

pub struct Miner {
    pub id: usize,
    pub rx: Receiver<Msg>,
    pub everybody: Broadband,
}

impl Miner {
    pub fn work(m: Miner) {
        let Miner {id: me, rx , mut everybody} = m;
        let mut total :u32 = 0;
        let mut current: u32 = 0;
        println!("{}: Journal start", me);
        while let Ok(msg) = rx.recv() {
            match msg {
                Msg::Return => {
                    total += current;
                    println!("{}: Return with {} gold found ", me, current);
                },
                Msg::YellGold {miner} => {
                    if miner == me {
                        everybody.cast(me, Msg::GoldFound{amount: current, miner: me});
                        println!("{}: GoldFound found {} ", me, current);
                    }
                },
                Msg::Work {region} => {
                    current = 0;
                    loop{
                        let mut amount = region.lock().unwrap();
                        if *amount <= 0 {
                            break;
                        } else {
                            *amount -= 1;
                            current += 1;
                        }
                        println!("{}: Working found gold +1", me);
                    }
                    println!("{}: Working found {} gold", me, current);
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
}

