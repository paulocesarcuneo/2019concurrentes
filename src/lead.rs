
use crate::utils::{GroupBy};
use crate::messages::*;
use std::sync::mpsc::{Receiver};
use std::sync::{Arc, Mutex};
use crate::logger::*;
use rand;

pub struct Lead {
    pub id: usize,
    pub rx: Receiver<Msg>,
    pub everybody: Broadband,
    pub regions: u32,
    pub logger:Logger,
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

fn lead_collect(rx: &Receiver<Msg>, active_miners: & Broadband, logger:&Logger) -> Vec<(u32,usize)> {
    let mut round_data = Vec::new();
    for (&m, tx)in active_miners {
        if m == 0 {
            continue;
        }
        tx.send(Msg::YellGold{miner: m}).unwrap();
        match rx.recv() {
            Ok(Msg::GoldFound {amount, miner})  => {
                round_data.push((amount, miner));
		logger.log(format!("{}: Recording {} gold found by {}", 0, amount, miner));
            },
            _ => {

                logger.log(format!("{}: unexpected message from {}", 0, m));
                break;
            }
        }
    }
    round_data
}

impl Lead {
    pub fn work(lead: Lead) {
        let Lead{id: me, mut everybody, rx, regions, logger} = lead;
        for r in 0..regions {
            if everybody.len() <= 1 {
                break;
            }
            let region = Arc::new(Mutex::new(rand::random::<u32>() % 100));
            everybody.cast(0, Msg::Work{region : region});
            everybody.cast(0, Msg::Return);
            let round_data = lead_collect(&rx, &everybody, &logger);
            let (winners, losers) = lead_round_results(&round_data);
            if losers.len() == 1 {
                let loser = losers[0];
                logger.log(format!("{}: Region {} Winners {:?} Losers {}", me, r, winners, loser));
                everybody.cast(0, Msg::RoundResult { winners : winners, losers : losers});
                everybody.remove(&loser);
            }
        }
        logger.log("lead: exit!".to_string());
        everybody.cast(0, Msg::Exit);
    }
}
