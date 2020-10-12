
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
#[macro_use]
extern crate log;
extern crate notify;

pub const LOG_LINE_LENGTH: usize = 384;

lazy_static! {
    // Log timestamp to position mapper
    pub static ref LOGS: RwLock<BTreeMap<u64, u64>> = RwLock::new(BTreeMap::new());
    pub static ref SEEKER: Seeker = {
        if cfg!(debug_assertions) {
            let mut args = env::args();
            args.next();
            Seeker::new(args.next().unwrap())
        } else {
            Seeker::new("/var/log/e-curtain.bin".to_string())
        }
    };
}

type FPData = f64;

#[derive(Deserialize)]
struct HistoryRequest {
    since: u64,
    until: u64,
}

#[get("/history")]
async fn history(info: web::Query<HistoryRequest>) -> Result<HttpResponse, Error> {
    let lines = SEEKER.read_log(info.since * 1000000, info.until * 1000000);
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
    env_logger::init();
    thread::spawn(move || SEEKER.watch_log());
    HttpServer::new(|| App::new()
        .wrap(middleware::Logger::default())
        .service(history)
        .service(current)
        .service(fs::Files::new("/", if cfg!(debug_assertions) {
            "."
        } else {
            "/var/lib/e-curtain/www"
        }).index_file("index.html")))
        .bind(if cfg!(debug_assertions) { "0.0.0.0:3000" } else { "0.0.0.0:80" })?
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
        info!("Read log from {}", filename);
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
        let mut watcher = watcher(tx, Duration::from_secs(1)).unwrap();
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
            let current_pos = meta.pos;
            meta.pathfinder.seek(io::SeekFrom::Start(current_pos)).unwrap();
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
    let res = unsafe {
        ptr::read_unaligned(&buffer as *const u8 as *const LogLine)
    };
    res
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
    slept: u64,
    offset: f64,
}

#[derive(Debug, Serialize, Deserialize)]
struct FuncDisp {
    state: String,
    slept: u64,
    offset: f64,
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
    dt: u64,
    r1: Room,
    r2: Room,
    r0: Ambient,
    f: FuncDisp,
    other: Other
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Other {
    acm: f64,
    acp: f64,
    reg1: f64,
    reg2: f64,
    fan: f64,
    win: f64,
    cur: f64,
    w0: f64,
    w1: f64,
    w2: f64,
    curbl: f64,
    curbu: f64,
    f012: f64,
    t0d: f64,
    f012bl: f64,
    f012bu: f64,
    ac1: f64,
    ac2: f64,
    t1m0: f64,
    Wsun: f64,
    qest: f64,
    cest: f64,
    tsr: f64,
    tss: f64
}

// Log representation of one line
// We don't bother byte orders for ptr::read will read the data in the exact ordering
#[derive(Debug)]
#[repr(align(1))]
pub struct LogLine {
    clk: u64,
    ar: LogAR,
    res_u: [f64; 8],
    res_par: [f64; 2],
    res_g: [f64; 7],
    res_q: [f64; 2],
    fr: Func,
}

#[derive(Debug)]
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
        let res = Historical {
            dt: self.clk / 1000000,
            t1: self.ar.uy0,
            tp1: self.ar.utp0,
            t2: self.ar.uy1,
            tp2: self.ar.utp1
        };
        res
    }
    fn to_current(&self) -> Current {
        let res = Current {
            dt: self.clk / 1000000,
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
            },
            other: Other {
                acm: self.res_g[2],
                acp: self.res_g[3],
                reg1: self.res_g[0],
                reg2: self.res_g[1],
                fan: self.res_g[4],
                win: self.res_g[5],
                cur: self.res_g[6],
                w0: self.ar.uw0,
                w1: self.ar.uw1,
                w2: self.ar.uw2,
                curbl: self.ar.ucurb0,
                curbu: self.ar.ucurb1,
                f012: self.res_u[2],
                t0d: self.res_u[3],
                f012bl: self.ar.uf012b0,
                f012bu: self.ar.uf012b1,
                ac1: self.res_u[0],
                ac2: self.res_u[1],
                t1m0: self.res_par[0],
                Wsun: self.res_par[1],
                qest: self.res_q[0],
                cest: self.res_q[1],
                tsr: self.ar.usun0,
                tss: self.ar.usun1
            }
        };
        res
    }
}
