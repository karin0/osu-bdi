use log::info;
use log::error;
use notify::{Watcher, RecursiveMode, RecommendedWatcher, Result, Event};
use crossbeam_channel::Sender;

pub fn watch(path: &str, tx: Sender<Event>) -> Result<RecommendedWatcher> {
    info!("Watching in {}", path);
    let mut watcher = notify::immediate_watcher(move |res| {
        match res {
            Ok(event) => {
                tx.send(event).unwrap()
            },
            Err(e) => error!("Error in watcher: {:?}", e)
        }
    })?;
    // watcher.configure(notify::Config::PreciseEvents(true))?;
    watcher.watch(path, RecursiveMode::NonRecursive)?;
    Ok(watcher)
}
