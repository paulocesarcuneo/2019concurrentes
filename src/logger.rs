use std::fs::{File, OpenOptions};
use std::io::BufWriter;
use std::io::Write;

static FILE_NAME: &'static str =  "result.txt";

pub fn open_log()-> File {

    // Open/create LOG_FILE for writing.
    let file = match OpenOptions::new()
		   .read(false)
            	   .append(true)
                   .write(true)
                   .create(true)
                   .open(FILE_NAME){
		   Err(_file) => panic!("Unable to open log file "),
		   Ok(file) => file,
		   };
    file
}

pub fn log(message: String) {

            let log_file = open_log();
            let mut log_writer = BufWriter::new(log_file);

	    match log_writer.write_all(message.as_bytes()){
		Err(_log_file) => panic!("Unable to writing log file "),
		Ok(_log_file) => ()
		}
	    
}

