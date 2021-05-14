mod watch;
mod handler;
mod dispatch;
mod win;

use log::error;
use log::info;

use dispatch::Event;
use handler::Handler;

use notify::Watcher;
use tungstenite::server::accept;
use env_logger::{Builder, Env};
use clap::Clap;
use chrono::Local;

use std::io::Write;
use std::net::TcpListener;
use std::sync::mpsc::{Sender, channel};
use std::thread::spawn;

fn listen(addr: &str, port: u16, tx: Sender<Event>) {
    let server = TcpListener::bind((addr, port)).unwrap();
    info!("Listening on {}:{}", addr, port);
    for stream in server.incoming() {
        if let Ok(stream) = stream {
            info!("New connection from {:?}", stream);
            match accept(stream) {
                Ok(conn) => {
                    tx.send(Event::Connection(conn)).unwrap();
                },
                Err(e) => {
                    error!("Failed to establish connection: {}", e)
                }
            }
        }
    }
}

#[derive(Clap)]
struct Opts {
    #[clap(short, long = "addr", default_value = "127.0.0.1")]
    addr: String,
    #[clap(short, long, default_value = "35677")]
    port: u16,

    #[clap(short, long)]
    songs_dir: Option<String>
}

fn main() {
    Builder::from_env(Env::default()
            .filter_or("BDI_LOG_LEVEL", "info")
            .write_style_or("BDI_LOG_STYLE", "never")
        )
        .format(|buf, rec| {
            writeln!(buf,
                "{} [{}] {} ({})",
                Local::now().format("%Y-%m-%d %H:%M:%S%z"),
                rec.level(),
                rec.args(),
                rec.module_path_static().unwrap_or("?")
            )
        })
        .init();

    let opts = Opts::parse();

    let (tx, rx) = channel();
    let path = &opts.songs_dir.unwrap_or_else(|| {
        match win::find_songs_path() {
            Some(s) => s,
            _ => {
                eprintln!("Cannot detect your osu! installation, please specify the Songs directory by --songs_path");
                std::process::exit(1);
            }
        }
    });
    let addr = opts.addr;
    let port = opts.port;
    // let path = "D:\\programs-local\\osu!\\Songs";

    let mut watcher = watch::watch(path, tx.clone()).unwrap();
    spawn(move || {
        listen(&addr, port, tx);
    });

    let mut handler = Handler::new(path).unwrap();
    dispatch::work(&mut handler, rx);
    watcher.unwatch(path).unwrap();
}
