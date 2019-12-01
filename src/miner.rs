
use crate::messages::*;
use std::convert::TryFrom;
use std::sync::mpsc::{Receiver};
use crate::logger::*;

pub struct Miner {
    pub id: usize,
    pub rx: Receiver<Msg>,
    pub everybody: Broadband,
    pub logger:Logger,
}

impl Miner {
    pub fn work(m: Miner) {
        let Miner {id: me, rx , mut everybody, logger} = m;
        let mut total :u32 = 0;
        let mut current: u32 = 0;
        logger.log(format!("{}: Journal start", me));
        while let Ok(msg) = rx.recv() {
            match msg {
                Msg::Return => {
                    total += current;
                    logger.log(format!("{}: Return with {} gold found ", me, current));
                },
                Msg::YellGold {miner} => {
                    if miner == me {
                        everybody.cast(me, Msg::GoldFound{amount: current, miner: me});
                        logger.log(format!("{}: GoldFound found {} ", me, current));
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
                        logger.log(format!("{}: Working found gold +1", me));
                    }
                    logger.log(format!("{}: Working found {} gold", me, current));
                },
                Msg::GoldFound {amount, miner}  => {
                    logger.log(format!("{}: I hear miner {} found {}", me, miner, amount))
                },
                Msg::RoundResult {winners, losers} => {
                    if losers.contains(&me) {
                        let transfer_amount = total / u32::try_from(winners.len()).unwrap();
                        for i in winners {
                            logger.log(format!("{}: Transfering {} to {}", me, transfer_amount, i));
                            everybody[&i].send(Msg::Transfer {amount: transfer_amount, sender: me, receiver: i}).unwrap();
                        }
                        logger.log(format!("{}: I'm fired :( . Bye... ", me));
                        break;
                    } else {
                        for l in losers {
                            everybody.remove(&l);
                        }
                    }
                },
                Msg::Transfer {amount, sender , receiver} => {
                    if receiver == me {
                        logger.log(format!("{}: Received {} from {}", me, amount, sender));
                        total += amount;
                    }
                },
                Msg::Exit => {
                    logger.log(format!("{}: Journal Done. Collected {} gold total", me, total));
                    break
                }
        }
        }
    }
}

