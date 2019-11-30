use std::sync::mpsc;
use std::thread;
use std::collections::{HashMap};
use std::env;

mod messages;
use messages::*;

mod miner;
use miner::*;

mod utils;

mod lead;
use lead::*;

fn show_help() {
    println!("Usage: ./concumining [MINERS] [REGIONS] [OPTIONS] \n");
    println!("\tOptions:");
    println!("\t\t -h \t\t Display this message");
    println!("\t\t -d \t\t Execution in debug mode");
    println!("\t\t -o FILENAME \t Output log file in debug mode");
}

fn run(regions: u32, miners: usize) {
// Initialization
    let mut txs = HashMap::new();
    let mut rxs = HashMap::new();
    for i in 0..miners+1 {
        let (tx, rx) = mpsc::channel();
        txs.insert(i, tx);
        rxs.insert(i, rx);
    };

    // Execution
    let mut ts = Vec::new();
    for i in 1..miners+1 {
        let rx = rxs.remove(&i).unwrap();
        let everybody : Broadband = txs
            .iter()
            .map(|a| (*a.0, a.1.clone()))
            .collect();
        let miner = Miner{id :i, rx: rx, everybody: everybody};
        let m = thread::spawn(move || {
            Miner::work(miner);
        });
        ts.push(m);
    };

    let everybody = txs;
    lead_work(0, rxs.remove(&0).unwrap(), everybody, regions);
    for t in ts {
        t.join().unwrap();
    }
    println!("main: done!");
}

fn main() {

    let _arguments: Vec<String> = env::args().collect();

    if _arguments.len() == 1 || _arguments.iter().position(|x| x == "-h").is_some() {
        show_help();
        return
    }

    let miners_arg : Option<&String> = _arguments.get(1);
    if miners_arg.is_none() {
        println!("\nMiners amount was not well specified!\n");
        show_help();
        return
    }

    if !miners_arg.unwrap().parse::<usize>().is_ok() {
        println!("\nThe miners amount specified was not a numeric value.\n");
        show_help();
        return
    }
    let miners_amount : usize = miners_arg.unwrap().parse::<usize>().unwrap();

    let regions_arg : Option<&String> = _arguments.get(2);
    if regions_arg.is_none() {
        println!("\nRegions amount was not well specified!\n");
        show_help();
        return
    }

    if !regions_arg.unwrap().parse::<u32>().is_ok() {
        println!("\nThe regions amount specified was not a numeric value.\n");
        show_help();
        return
    }
    let regions_amount : u32  = miners_arg.unwrap().parse::<u32>().unwrap();

    //TODO: integrate debug options
    let debug_idx : Option<usize> = _arguments.iter().position(|x| x == "-d");
    if debug_idx.is_some() {

        let mut output_filepath : &String = &"log.txt".to_string();
        let output_idx : Option<usize> = _arguments.iter().position(|x| x == "-o");
        if output_idx.is_some() {
            output_filepath = _arguments.get(output_idx.unwrap() + 1).unwrap();
        }
    }

    run(regions_amount, miners_amount);
    /* const R :u32 = 10;
    const M :usize = 10;

    run(R, M); */
}
