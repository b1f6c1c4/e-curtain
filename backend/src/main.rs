use actix_files as fs;
use actix_web::{get, middleware, post, web, App, Error, HttpResponse, HttpServer};
use notify::{watcher, RecursiveMode, Watcher};
use num_enum::TryFromPrimitive;
use parking_lot::*;
use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;
use std::convert::TryFrom;
use std::env;
use std::fs::File;
use std::io;
use std::io::prelude::*;
use std::io::Read;
use std::string::ToString;
use std::sync::mpsc::channel;
use std::thread;
use std::time::Duration;
use std::{mem, ptr};
use strum_macros;
use tokio::net::UdpSocket;

#[macro_use]
extern crate lazy_static;
#[macro_use]
extern crate log;
extern crate notify;

pub const LOG_LINE_LENGTH: usize = 448;

lazy_static! {
    // Log timestamp to position mapper
    pub static ref LOGS: RwLock<BTreeMap<u64, u64>> = RwLock::new(BTreeMap::new());
    pub static ref SEEKER: Seeker = Seeker::new(env::args().nth(1).unwrap());
    pub static ref UDP_ADDR: String = format!("{}:33706", env::args().nth(2).unwrap());
}

#[post("/offset2")]
async fn offset2(cmd: web::Json<OffsetCmd>) -> Result<HttpResponse, Error> {
    let mut socket = UdpSocket::bind("0.0.0.0:0").await?;
    let nan = std::f64::NAN;
    let v = cmd.cmd;
    let data = [nan, v, nan, nan];
    let data_packet = unsafe { &mem::transmute::<_, [u8; 4 * 8]>(data) };
    socket.connect(&*UDP_ADDR).await?;
    socket.send(&data_packet[..]).await?;
    Ok(HttpResponse::Ok().json(()))
}

#[get("/history")]
async fn history(info: web::Query<HistoryRequest>) -> Result<HttpResponse, Error> {
    let lines = SEEKER.read_log(info.since * 1000000, info.until * 1000000);
    let data = lines
        .into_iter()
        .map(|l| l.to_history())
        .collect::<Vec<_>>();
    Ok(HttpResponse::Ok().json(data))
}

#[get("/current")]
async fn current() -> Result<HttpResponse, Error> {
    let last = SEEKER.last();
    let data = last.map(|l| l.to_current());
    Ok(HttpResponse::Ok().json(data))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    env_logger::init();
    thread::spawn(move || SEEKER.watch_log());
    HttpServer::new(|| {
        App::new()
            .wrap(middleware::Logger::default())
            .service(history)
            .service(current)
            .service(offset2)
            .service(
                fs::Files::new(
                    "/",
                    if cfg!(debug_assertions) {
                        "."
                    } else {
                        "/var/lib/e-curtain/www"
                    },
                )
                .index_file("index.html"),
            )
    })
    .bind(if cfg!(debug_assertions) {
        "0.0.0.0:3000"
    } else {
        "0.0.0.0:80"
    })?
    .run()
    .await
}

pub struct Seeker {
    filename: String,
    meta: RwLock<SeekerMeta>,
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
                pos: 0,
            }),
            filename,
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
        watcher
            .watch(&self.filename, RecursiveMode::NonRecursive)
            .unwrap();
        loop {
            match rx.recv() {
                Ok(_) => {
                    self.refresh_logs();
                }
                Err(e) => panic!("{:?}", e),
            }
        }
    }
    fn refresh_logs(&self) {
        loop {
            let mut line_buffer = [0u8; LOG_LINE_LENGTH];
            let mut meta = self.meta.write();
            let current_pos = meta.pos;
            meta.pathfinder
                .seek(io::SeekFrom::Start(current_pos))
                .unwrap();
            match meta.pathfinder.read_exact(&mut line_buffer) {
                Ok(_) => {
                    let timestamp =
                        unsafe { ptr::read_unaligned(&line_buffer as *const u8 as *const u64) };
                    LOGS.write().insert(timestamp, meta.pos);
                    meta.pos += LOG_LINE_LENGTH as u64;
                }
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
            if !log_line.fr.state.is_nan() {
                res.push(log_line);
            }
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
    let res = unsafe { ptr::read_unaligned(&buffer as *const u8 as *const LogLine) };
    res
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Historical {
    dt: u64,  // unix timestamp in seconds
    t1: f64,  // actual temperature of room 1
    tp1: f64, // desired temperature of room 1
    t2: f64,  // actual temperature of room 2
    tp2: f64, // desired temperature of room 2
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Room {
    t: f64,  // actual room temperature
    tp: f64, // desired room temperature
    h: f64,  // relative roome humidity
}
#[derive(Debug, Serialize, Deserialize)]
struct Ambient {
    t: f64, // temperature
    h: f64, // humidity
    uvi: f64,
    clouds: f64,
    wind: f64,
}

#[derive(Debug, Serialize, Deserialize)]
struct OtherRecord {
    state: f64,
    slept: f64,
    offset: f64,
    uvi: f64,
    clouds: f64,
    wind: f64,
    offset2: f64,
}

#[derive(Debug, Serialize, Deserialize)]
struct FuncDisp {
    state: String,
    slept: f64,
    offset: f64,
    offset2: f64,
}

#[derive(Eq, PartialEq, TryFromPrimitive)]
#[repr(usize)]
#[derive(strum_macros::ToString, Debug)]
enum FuncState {
    NOBODY,
    NORMAL,
    SNAP,
    SLEEP,
    RSNAP,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Current {
    dt: u64,
    r1: Room,
    r2: Room,
    r0: Ambient,
    f: FuncDisp,
    other: Other,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Other {
    heat: f64,
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
    tss: f64,
}

// Log representation of one line
// We don't bother byte orders for ptr::read will read the data in the exact ordering
#[derive(Debug)]
#[repr(align(1))]
pub struct LogLine {
    clk: u64,
    ar: LogAR,
    res_u: [f64; 9],
    res_par: [f64; 2],
    res_g: [f64; 8],
    res_q: [f64; 2],
    fr: OtherRecord,
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
    uw2: f64,
}

impl LogLine {
    fn to_history(&self) -> Historical {
        let res = Historical {
            dt: self.clk / 1000000,
            t1: self.ar.uy0,
            tp1: self.ar.utp0,
            t2: self.ar.uy1,
            tp2: self.ar.utp1,
        };
        res
    }
    fn to_current(&self) -> Current {
        let res = Current {
            dt: self.clk / 1000000,
            r1: Room {
                t: self.ar.uy0,
                tp: self.ar.utp0,
                h: self.ar.h1,
            },
            r2: Room {
                t: self.ar.uy1,
                tp: self.ar.utp1,
                h: self.ar.h2,
            },
            r0: Ambient {
                t: self.ar.ut0,
                h: self.ar.h0,
                uvi: self.fr.uvi,
                clouds: self.fr.clouds,
                wind: self.fr.wind,
            },
            f: FuncDisp {
                state: FuncState::try_from(self.fr.state as usize)
                    .unwrap()
                    .to_string(),
                offset: self.fr.offset,
                offset2: self.fr.offset2,
                slept: self.fr.slept,
            },
            other: Other {
                heat: self.res_g[0],
                acm: self.res_g[3],
                acp: self.res_g[4],
                reg1: self.res_g[1],
                reg2: self.res_g[2],
                fan: self.res_g[5],
                win: self.res_g[6],
                cur: self.res_g[7],
                w0: self.ar.uw0,
                w1: self.ar.uw1,
                w2: self.ar.uw2,
                curbl: self.ar.ucurb0,
                curbu: self.ar.ucurb1,
                f012: self.res_u[3],
                t0d: self.res_u[4],
                f012bl: self.ar.uf012b0,
                f012bu: self.ar.uf012b1,
                ac1: self.res_u[1],
                ac2: self.res_u[2],
                t1m0: self.res_par[0],
                Wsun: self.res_par[1],
                qest: self.res_q[0],
                cest: self.res_q[1],
                tsr: self.ar.usun0,
                tss: self.ar.usun1,
            },
        };
        res
    }
}

#[derive(Debug, Serialize, Deserialize)]
struct OffsetCmd {
    cmd: f64,
}

#[derive(Debug, Serialize, Deserialize)]
struct HistoryRequest {
    since: u64,
    until: u64,
}
