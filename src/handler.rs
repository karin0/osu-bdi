use log::debug;
use log::info;

use tungstenite::{Message, WebSocket};

use std::collections::HashSet;
use std::ffi::OsStr;
use std::fs::read_dir;
use std::io;
use std::net::TcpStream;
use std::path::Path;

pub type Conn = WebSocket<TcpStream>;

pub fn to_id(s: &OsStr) -> Option<String> {
    let r: String = s
        .to_string_lossy()
        .chars()
        .take_while(char::is_ascii_digit)
        .collect();
    if r.is_empty() {
        return None;
    }
    Some(r)
}

type Id = u32;

pub fn to_id_int(s: &OsStr) -> Option<Id> {
    s.to_string_lossy()
        .chars()
        .take_while(char::is_ascii_digit)
        .map(|c| c as Id - '0' as Id)
        .reduce(|a, b| a * 10 + b)
}

pub struct Handler {
    conns: Vec<Conn>,
    ids: HashSet<Id>,
}

impl Handler {
    fn send(&mut self, msg: &str) {
        self.conns
            .retain_mut(|ws: &mut Conn| match ws.send(Message::text(msg)) {
                Err(e) => {
                    debug!("Disconnecting: {:?}: {}", ws.get_ref(), e);
                    false
                }
                _ => true,
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
        for id in &ids {
            info!("Adding {id}");
            self.ids.insert(id.parse::<Id>().unwrap());
        }
        self.notify(ids, '+');
    }

    pub fn remove(&mut self, ids: Vec<String>) {
        for id in &ids {
            info!("Removing {id}");
            self.ids.remove(&id.parse::<Id>().unwrap());
        }
        self.notify(ids, '-');
    }

    pub fn add_conn(&mut self, mut conn: Conn) {
        let mut msg = String::from('+');
        for id in &self.ids {
            msg.push(' ');
            msg.push_str(&id.to_string());
        }
        if conn.send(Message::text(msg)).is_ok() {
            self.conns.push(conn);
        }
    }

    pub fn from(path: &Path) -> io::Result<Handler> {
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
            ids,
        })
    }
}
