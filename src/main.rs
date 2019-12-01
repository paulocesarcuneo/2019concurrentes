use std::sync::mpsc;
use std::thread;
use std::collections::{HashMap};

mod messages;
use messages::*;

mod miner;
use miner::*;

mod utils;

mod lead;
use lead::*;

mod logger;
use logger::*;

fn main() {
    const R :u32 = 10;
    const M :usize = 10;

    // Initialization
    log("Open log file".to_string()); 
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
    log("main: done!".to_string());
}
