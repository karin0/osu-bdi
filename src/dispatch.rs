use crate::handler::{self, Handler, Conn};

use log::debug;
use log::info;

use notify::{Event, EventKind};
use notify::event::{CreateKind, ModifyKind, RemoveKind, RenameMode};
use crossbeam_channel::{select, Receiver};
use lazy_static::lazy_static;

use std::path::PathBuf;
use std::ffi::OsStr;

fn to_id(path: PathBuf) -> Option<String> {
    path.file_name().and_then(handler::to_id)
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

lazy_static! {
    static ref SOME_OSZ: Option<&'static OsStr> = Some(OsStr::new("osz"));
}

fn dispatch_remove(handler: &mut Handler, paths: Vec<PathBuf>) {
    if !paths.is_empty() {
        let paths: Vec<String> = paths.into_iter().filter(|buf| {
            buf.extension() != *SOME_OSZ
        }).filter_map(to_id).collect();
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

pub fn work(hdr: &mut Handler, fs_rx: Receiver<Event>, conn_rx: Receiver<Conn>) {
    info!("Worker started");
    loop {
        select! {
            recv(fs_rx) -> res => match res {
                Ok(event) => {
                    debug!("Fs event: {:?}", event);
                    let paths = event.paths;
                    match event.kind {
                        EventKind::Create(CreateKind::Any) |
                        EventKind::Create(CreateKind::Folder) |
                        EventKind::Modify(ModifyKind::Name(RenameMode::To))
                            => dispatch_create(hdr, paths),
                        EventKind::Remove(RemoveKind::Any) |
                        EventKind::Remove(RemoveKind::Folder) |
                        EventKind::Modify(ModifyKind::Name(RenameMode::From))
                            => dispatch_remove(hdr, paths),
                        EventKind::Modify(ModifyKind::Name(RenameMode::Both))
                            => dispatch_rename(hdr, paths),
                        _ => ()
                    }
                },
                _ => {
                    return;
                }
            },
            recv(conn_rx) -> res => match res {
                Ok(conn) =>
                    hdr.add_conn(conn),
                _ => {
                    return;
                }
            }
         }
    }
}
