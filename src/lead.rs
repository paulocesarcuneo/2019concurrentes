
use crate::utils::{GroupBy};
use crate::messages::*;
use std::sync::mpsc::{Receiver};

use crate::logger::log;

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
		log(format!("{}: Recording {} gold found by {}", 0, amount, miner));
            },
            _ => {
                log("Err".to_string());
                break;
            }
        }
    }
    round_data
}

pub fn lead_work(me: usize, rx: Receiver<Msg>, mut everybody: Broadband, regions: u32) {
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
            log(format!("{}: Region {} Winners {:?} Losers {}", me, r, winners, loser)) ;
            everybody.cast(0, Msg::RoundResult { winners : winners, losers : losers});
            everybody.remove(&loser);
        }
    }
    log("lead: exit!".to_string());
    everybody.cast(0, Msg::Exit);

}
