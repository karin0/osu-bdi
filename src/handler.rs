use log::info;

use tungstenite::{Message, WebSocket};
use retain_mut::RetainMut;

use std::io;
use std::fs::read_dir;
use std::ffi::OsStr;
use std::net::TcpStream;
use std::collections::HashSet;

pub type Conn = WebSocket<TcpStream>;

pub fn to_id(s: &OsStr) -> Option<String> {
    let s = s.to_string_lossy();
    let mut it = s.chars();
    let mut r = String::new();
    loop {
        if let Some(c) = it.next() {
            if c.is_ascii_digit() {
                r.push(c);
                continue;
            }
        }
        break;
    }
    if r.is_empty() {
        return None;
    }
    Some(r)
}

pub fn to_id_int(s: &OsStr) -> Option<u32> {
    let s = s.to_string_lossy();
    let mut it = s.chars();
    let mut r;
    if let Some(c) = it.next() {
        if c.is_ascii_digit() {
            r = c.to_digit(10).unwrap();
        } else {
            return None;
        }
    } else {
        return None;
    }
    loop {
        if let Some(c) = it.next() {
            if c.is_ascii_digit() {
                r = r * 10 + c.to_digit(10).unwrap();
                continue;
            }
        }
        break;
    }
    Some(r)
}

pub struct Handler {
    conns: Vec<Conn>,
    ids: HashSet<u32>
}

impl Handler {
    fn send(&mut self, msg: &str) {
        self.conns.retain_mut(|ws: &mut Conn| {
            let r = ws.can_write() && ws.write_message(Message::text(msg)).is_ok();
            if !r {
                info!("Dropping connection: {:?}", ws);
            }
            r
        });
    }

    fn notify(&mut self, ids: Vec<String>, kind: char) {
        let mut msg = String::from(kind);
        for id in ids {
            msg.push(' ');
            msg.push_str(&id);
        }
        self.send(&msg);
    }

    pub fn create(&mut self, ids: Vec<String>) {
        info!("Adding beatmap(s): {:?}", ids);
        for id in &ids {
            self.ids.insert(id.parse::<u32>().unwrap());
        }
        self.notify(ids, '+')
    }

    pub fn remove(&mut self, ids: Vec<String>) {
        info!("Removing beatmap(s): {:?}", ids);
        for id in &ids {
            self.ids.remove(&id.parse::<u32>().unwrap());
        }
        self.notify(ids, '-')
    }

    pub fn add_conn(&mut self, mut conn: Conn) {
        let mut msg = String::from('+');
        for id in &self.ids {
            msg.push(' ');
            msg.push_str(&id.to_string());
        }
        if conn.write_message(Message::text(msg)).is_ok() {
            self.conns.push(conn);
        }
    }

    pub fn from(path: &str) -> io::Result<Handler> {
        let mut ids = HashSet::new();
        for ent in read_dir(path)? {
            let ent = ent?;
            if ent.file_type()?.is_dir() {
                if let Some(id) = to_id_int(&ent.file_name()) {
                    ids.insert(id);
                }
            }
        }
        info!("Found {} beatmap(s)", ids.len());

        Ok(Handler {
            conns: Vec::new(),
            ids
        })
    }
}
