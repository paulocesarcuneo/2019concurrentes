use std::sync::mpsc::{Sender};
use std::collections::{HashMap};

#[derive(Clone, Debug)]
pub enum Msg{
    Return,
    Work {region : u32},
    GoldFound {amount: u32, miner: usize},
    RoundResult {winners: Vec<usize>, losers: Vec<usize> },
    Transfer {amount: u32, sender:usize, receiver: usize},
    Exit,
}

pub type Broadband = HashMap<usize, Sender<Msg>>;

pub trait Broadcast {
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
