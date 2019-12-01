
use crate::messages::*;
use rand;
use std::convert::TryFrom;
use std::sync::mpsc::{Receiver};
use crate::logger::log;

pub fn miner_work(me: usize, rx: Receiver<Msg>, mut everybody: Broadband)  {
    let mut total :u32 = 0;
    log(format!("{}: Journal start", me));
    while let Ok(msg) = rx.recv() {
        match msg {
            Msg::Return => {
                let amount = rand::random::<u32>() % 10;
                total += amount;
                everybody.cast(me, Msg::GoldFound{amount, miner: me});
                log(format!("{}: Return with {} gold found ", me, amount))
            },
            Msg::Work {region} => {
                log(format!("{}: Working region {}", me, region))
            },
            Msg::GoldFound {amount, miner}  => {
                log(format!("{}: I hear miner {} found {}", me, miner, amount))
            },
            Msg::RoundResult {winners, losers} => {
                if losers.contains(&me) {
                    let transfer_amount = total / u32::try_from(winners.len()).unwrap();
                    for i in winners {
                        log(format!("{}: Transfering {} to {}", me, transfer_amount, i));
                        everybody[&i].send(Msg::Transfer {amount: transfer_amount, sender: me, receiver: i}).unwrap();
                    }
                    log(format!("{}: I'm fired :( . Bye... ", me));
                    break;
                } else {
                    for l in losers {
                        everybody.remove(&l);
                    }
                }
            },
            Msg::Transfer {amount, sender , receiver} => {
                if receiver == me {
                    log(format!("{}: Received {} from {}", me, amount, sender));
                    total += amount;
                }
            },
            Msg::Exit => {
                log(format!("{}: Journal Done. Collected {} gold total", me, total));
                break
            }
        }
    }
}
