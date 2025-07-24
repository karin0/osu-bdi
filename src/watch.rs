use std::path::Path;

use crossbeam_channel::Sender;
use log::error;
use log::info;
use notify::{Event, RecommendedWatcher, RecursiveMode, Result, Watcher};

pub fn watch(path: &Path, tx: Sender<Event>) -> Result<RecommendedWatcher> {
    info!("Watching in {}", path.display());
    let mut watcher = notify::recommended_watcher(move |res| match res {
        Ok(event) => tx.send(event).unwrap(),
        Err(e) => error!("Error in watcher: {e:?}"),
    })?;
    // watcher.configure(notify::Config::PreciseEvents(true))?;
    watcher.watch(path, RecursiveMode::NonRecursive)?;
    Ok(watcher)
}
