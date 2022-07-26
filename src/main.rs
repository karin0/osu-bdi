mod watch;
mod handler;
mod dispatch;
mod win;

use handler::{Handler, Conn};
use log::error;
use log::info;

use notify::Watcher;
use tungstenite::accept;
use env_logger::{Builder, Env};
use clap::Parser;
use chrono::Local;
use crossbeam_channel::{Sender, unbounded};

use std::io::Write;
use std::net::TcpListener;
use std::path::PathBuf;
use std::thread::spawn;
use std::panic;
use std::process;
use std::path::Path;

fn listen(addr: &str, port: u16, tx: Sender<Conn>) {
    let server = TcpListener::bind((addr, port)).unwrap();
    info!("Listening on {}", server.local_addr().unwrap());
    for stream in server.incoming().flatten() {
        if let Ok(addr) = stream.peer_addr() {
            info!("Connecting: {}", addr);
            match accept(stream) {
                Ok(conn) => {
                    tx.send(conn).unwrap();
                },
                Err(e) => {
                    error!("Websocket handshake failed: {}", e)
                }
            }
        }
    }
}

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
struct Opts {
    #[clap(short, long = "addr", default_value = "127.0.0.1")]
    addr: String,
    #[clap(short, long, default_value = "35677")]
    port: u16,

    #[clap(short, long)]
    songs_dir: Option<PathBuf>
}

fn main() {
    Builder::from_env(
            Env::default()
                .filter_or("BDI_LOG_LEVEL", "info")
                .write_style_or("BDI_LOG_STYLE", "never")
        )
        .format(|buf, rec| {
            writeln!(buf,
                "{} [{}] {} ({})",
                Local::now().format("%Y-%m-%d %H:%M:%S%.3f%z"),
                rec.level(),
                rec.args(),
                rec.module_path_static().unwrap_or("?")
            )
        })
        .init();

    let opts = Opts::parse();
    let path: &Path = match &opts.songs_dir {
        Some(s) => s,
        None => {
            let t = Box::leak(win::find_songs_path().unwrap_or_else(|| {
                eprintln!("Cannot detect your osu! installation, please specify your Songs directory by --songs-dir");
                process::exit(1);
            }).into_boxed_str());
            (*t).as_ref()
        }
    };
    let addr = opts.addr;
    let port = opts.port;

    let hook = panic::take_hook();
    panic::set_hook(Box::new(move |info| {
        hook(info);
        process::exit(1);
    }));

    let (fs_tx, fs_rx) = unbounded();
    let (conn_tx, conn_rx) = unbounded();
    let mut watcher = watch::watch(&path, fs_tx).unwrap();
    spawn(move || {
        listen(&addr, port, conn_tx);
    });

    let mut handler = Handler::from(&path).unwrap();
    dispatch::work(&mut handler, fs_rx, conn_rx);
    watcher.unwatch(&path).unwrap();
}
