use crate::handler::{self, Handler, Conn};

use log::debug;
use log::info;

use notify::{EventKind};
use notify::event::{CreateKind, ModifyKind, RemoveKind, RenameMode};
use std::path::PathBuf;
use std::sync::mpsc::Receiver;

fn to_id(path: PathBuf) -> Option<String> {
    match path.file_name() {
        Some(name) => handler::to_id(name),
        None => None
    }
}

pub enum Event {
    Fs(notify::Event),
    Connection(Conn)
}

fn dispatch_create(handler: &mut Handler, mut paths: Vec<PathBuf>) {
    paths.retain(|p| p.is_dir());
    if !paths.is_empty() {
        let paths: Vec<String> = paths.into_iter().filter_map(to_id).collect();
        if !paths.is_empty() {
            handler.create(paths);
        }
    }
}

fn dispatch_remove(handler: &mut Handler, paths: Vec<PathBuf>) {
    if !paths.is_empty() {
        let paths: Vec<String> = paths.into_iter().filter_map(to_id).collect();
        if !paths.is_empty() {
            handler.remove(paths);
        }
    }
}

fn dispatch_rename(handler: &mut Handler, paths: Vec<PathBuf>) {
    let mut it = paths.into_iter();
    if let Some(src) = it.next() {
        if let Some(dst) = it.next() {
            if let Some(srcid) = to_id(src) {
                handler.remove(vec![srcid]);
            }
            if let Some(dstid) = to_id(dst) {
                handler.create(vec![dstid]);
            }
        }
    }
}

pub fn work(hdr: &mut Handler, rx: Receiver<Event>) {
    info!("Worker started");
    loop {
        match rx.recv() {
            Ok(Event::Fs(event)) => {
                debug!("fs event: {:?}", event);
                let paths = event.paths;
                match event.kind {
                    EventKind::Create(CreateKind::Any) => dispatch_create(hdr, paths),
                    EventKind::Create(CreateKind::Folder) => dispatch_create(hdr, paths),
                    EventKind::Modify(ModifyKind::Name(RenameMode::From)) => dispatch_remove(hdr, paths),
                    EventKind::Modify(ModifyKind::Name(RenameMode::To)) => dispatch_create(hdr, paths),
                    EventKind::Modify(ModifyKind::Name(RenameMode::Both)) => dispatch_rename(hdr, paths),
                    EventKind::Remove(RemoveKind::Any) => dispatch_remove(hdr, paths),
                    EventKind::Remove(RemoveKind::Folder) => dispatch_remove(hdr, paths),
                    _ => {}
                }
            },
            Ok(Event::Connection(conn)) => {
                hdr.add_conn(conn);
            },
            _ => {
                return;
            }
         }
    }
}
