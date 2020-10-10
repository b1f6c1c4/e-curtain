
use actix_web::{
    error, middleware, web, App, Error, HttpRequest, HttpResponse, HttpServer, get, post, Responder
};
use actix_files as fs;
use notify::{Watcher, RecursiveMode, watcher};
use std::sync::mpsc::channel;
use std::time::Duration;
use serde::{Deserialize, Serialize};
use std::{mem, ptr};
use std::collections::BTreeMap;
use std::io;
use std::io::prelude::*;
use std::thread;
use std::env;
use std::fs::File;
use parking_lot::*;
use std::string::ToString;
use strum_macros;
use num_enum::TryFromPrimitive;
use std::convert::TryFrom;

#[macro_use]
extern crate lazy_static;
extern crate notify;

pub const LOG_LINE_LENGTH: usize = 384;

lazy_static! {
    // Log timestamp to position mapper
    pub static ref LOGS: RwLock<BTreeMap<u64, u64>> = RwLock::new(BTreeMap::new());
    pub static ref SEEKER: Seeker = Seeker::new(env::args().next().unwrap());
}

type FPData = f64;

#[get("/history")]
async fn history(web::Query((since, until)): web::Query<(u64, u64)>) -> Result<HttpResponse, Error>  {
    let lines = SEEKER.read_log(since, until);
    let data = lines.into_iter().map(|l| l.to_history()).collect::<Vec<_>>();
    Ok(HttpResponse::Ok().json(data))
}

#[get("/current")]
async fn current() -> Result<HttpResponse, Error>  {
    let last = SEEKER.last();
    let data = last.map(|l| l.to_current());
    Ok(HttpResponse::Ok().json(data))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    thread::spawn(move || SEEKER.watch_log());
    HttpServer::new(|| App::new()
        .wrap(middleware::Logger::default())
        .service(history)
        .service(current)
        .service(fs::Files::new("/", "/var/lib/e-curtain/www").index_file("index.html")))
        .bind("0.0.0.0:3000")?
        .run()
        .await
}

pub struct Seeker {
    filename: String,
    meta: RwLock<SeekerMeta>
}

struct SeekerMeta {
    pathfinder: io::BufReader<File>,
    pos: u64,
}

impl Seeker {
    fn new(filename: String) -> Seeker {
        let obj = Self {
            meta: RwLock::new(SeekerMeta {
                pathfinder: io::BufReader::new(File::open(&filename).unwrap()),
                pos: 0
            }),
            filename
        };
        obj.refresh_logs();
        obj
    }
    fn watch_log(&self) -> io::Result<()> {
        // Create a channel to receive the events.
        let (tx, rx) = channel();

        // Create a watcher object, delivering debounced events.
        // The notification back-end is selected based on the platform.
        let mut watcher = watcher(tx, Duration::from_secs(10)).unwrap();
        watcher.watch(&self.filename, RecursiveMode::NonRecursive).unwrap();
        loop {
            match rx.recv() {
                Ok(_) => {
                    self.refresh_logs();
                },
                Err(e) => panic!("{:?}", e)
            }
        }
    }
    fn refresh_logs(&self) {
        loop {
            let mut line_buffer = [0u8; LOG_LINE_LENGTH];
            let mut meta = self.meta.write();
            match meta.pathfinder.read_exact(&mut line_buffer) {
                Ok(_) => {
                    let timestamp = unsafe {
                        ptr::read_unaligned(&line_buffer as *const u8 as *const u64)
                    };
                    LOGS.write().insert(timestamp, meta.pos);
                    meta.pos += LOG_LINE_LENGTH as u64;
                },
                Err(e) => {
                    if e.kind() == io::ErrorKind::UnexpectedEof {
                        break;
                    } else {
                        panic!("{:?}", e);
                    }
                }
            }
        }
    }
    fn read_log(&self, start: u64, end: u64) -> Vec<LogLine> {
        let mut res = vec![];
        let mut file = File::open(&self.filename).unwrap();
        let cursor = LOGS.read();
        let range = cursor.range(start..end);
        for (t, pos) in range {
            file.seek(io::SeekFrom::Start(*pos)).unwrap();
            let log_line = read_to_line(&mut file);
            assert_eq!(log_line.clk, *t);
            res.push(log_line);
        }
        res
    }
    fn last(&self) -> Option<LogLine> {
        let mut file = File::open(&self.filename).unwrap();
        if let Some((t, pos)) = LOGS.read().iter().last() {
            file.seek(io::SeekFrom::Start(*pos)).unwrap();
            let log_line = read_to_line(&mut file);
            assert_eq!(log_line.clk, *t);
            Some(log_line)
        } else {
            None
        }
    }
}

fn read_to_line(file: &mut File) -> LogLine {
    let mut buffer = [0u8; LOG_LINE_LENGTH];
    file.read_exact(&mut buffer).unwrap();
    unsafe {
        ptr::read_unaligned(&buffer as *const u8 as *const LogLine)
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Historical {
    dt: u64, // unix timestamp in seconds
    t1: FPData, // actual temperature of room 1
    tp1: FPData, // desired temperature of room 1
    t2: FPData, // actual temperature of room 2
    tp2: FPData, // desired temperature of room 2
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Room {
    t: FPData, // actual room temperature
    tp: FPData, // desired room temperature
    h: FPData, // relative roome humidity
}
#[derive(Debug, Serialize, Deserialize)]
struct Ambient {
    t: FPData, // temperature
    h: FPData, // humidity
    uvi: FPData, //
    wind: FPData
}

#[derive(Debug, Serialize, Deserialize)]
struct Func {
    state: f64,
    offset: f64,
    slept: u64,
}

#[derive(Debug, Serialize, Deserialize)]
struct FuncDisp {
    state: String,
    offset: f64,
    slept: u64,
}

#[derive(Eq, PartialEq, TryFromPrimitive)]
#[repr(usize)]
#[derive(strum_macros::ToString, Debug)]
enum FuncState {
    NOBODY,
    NORMAL,
    SNAP,
    SLEEP,
    RSNAP
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Current {
    r1: Room,
    r2: Room,
    r0: Ambient,
    f: FuncDisp
}

// Log representation of one line
// We don't bother byte orders for ptr::read will read the data in the exact ordering
#[repr(align(1))]
pub struct LogLine {
    clk: u64,
    ar: LogAR,
    res: [u8; 19],
    fr: Func,
    pad: [u8; 40]
}

#[repr(align(1))]
pub struct LogAR {
    ut0: f64,
    uy0: f64,
    uy1: f64,
    utp0: f64,
    utp1: f64,
    h0: f64,
    h1: f64,
    h2: f64,
    usun0: f64,
    usun1: f64,
    usun2: f64,
    f012bu0: f64,
    f012bu1: f64,
    uf012b0: f64,
    uf012b1: f64,
    ucurb0: f64,
    ucurb1: f64,
    uw0: f64,
    uw1: f64,
    uw2: f64
}

impl LogLine {
    fn to_history(&self) -> Historical {
        Historical {
            dt: self.clk,
            t1: self.ar.uy0,
            tp1: self.ar.utp0,
            t2: self.ar.uy1,
            tp2: self.ar.utp1
        }
    }
    fn to_current(&self) -> Current {
        Current {
            r1: Room {
                t: self.ar.uy0,
                tp: self.ar.utp0,
                h: self.ar.h1
            },
            r2: Room {
                t: self.ar.uy1,
                tp: self.ar.utp1,
                h: self.ar.h2
            },
            r0: Ambient {
                t: self.ar.ut0,
                h: self.ar.h0,
                uvi: self.ar.usun2,
                wind: 0f64 // TODO: fill this with actual data
            },
            f: FuncDisp {
                state: FuncState::try_from(self.fr.state as usize).unwrap().to_string(),
                offset: self.fr.offset,
                slept: self.fr.slept
            }
        }
    }
}