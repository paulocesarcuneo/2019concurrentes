
use crate::messages::*;
use rand;
use std::convert::TryFrom;
use std::sync::mpsc::{Receiver};

pub fn miner_work(me: usize, rx: Receiver<Msg>, mut everybody: Broadband)  {
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
