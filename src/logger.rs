use std::fs::{File, OpenOptions};
use std::io::BufWriter;
use std::io::Write;
use std::sync::{Arc, Mutex};

pub trait Logging {
    fn log(&self, msg: String);
}

pub type Logger = Arc<Mutex<BufWriter<File>>>;
impl Logging for Logger {
    fn log(&self, message: String) {
        match self
            .lock()
            .unwrap()
            .write(format!("{}\n", message).as_bytes())
        {
            Err(_log_file) => panic!("Unable to writing log file "),
            Ok(_log_file) => (),
        }
    }
}

pub fn open_log(log_path: &String) -> Logger {
    // Open/create LOG_FILE for writing.
    let file = match OpenOptions::new()
        .read(false)
        .append(true)
        .write(true)
        .create(true)
        .open(log_path)
    {
        Err(_file) => panic!("Unable to open log file "),
        Ok(file) => file,
    };
    Arc::new(Mutex::new(BufWriter::new(file)))
}
