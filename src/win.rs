use log::error;
use log::info;
use log::debug;

use winreg::enums::HKEY_CLASSES_ROOT;
use winreg::RegKey;

use std::io;
use std::path::PathBuf;

fn get_osu_command() -> io::Result<String> {
    let it = RegKey::predef(HKEY_CLASSES_ROOT)
        .open_subkey("osu\\shell\\open\\command")?;
    let s: String = it.get_value("")?;
    Ok(s)
}

fn get_osu_path() -> Option<String> {
    match get_osu_command() {
        Ok(s) => {
            debug!("Found in registry: {}", s);
            match s.chars().next() {
                Some('"') => {
                    match s[1..].find('"') {
                        Some(p) => Some(s[1..p + 1].to_owned()),
                        _ => None
                    }
                },
                _ => match s.find(' ') {
                    Some(p) => Some(s[0..p].to_owned()),
                    _ => Some(s)
                }
            }
        },
        Err(e) => {
            error!("Error reading registy: {:?}", e);
            None
        }
    }
}

pub fn find_songs_path() -> Option<String> {
    get_osu_path().map(|path| {
        info!("Found osu! installation at {}", path);
        let mut path = PathBuf::from(path);
        path.set_file_name("Songs");
        path.to_str().unwrap().to_owned()
    })
}
