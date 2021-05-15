#[cfg(windows)]
mod sys {
    use log::error;
    use log::info;
    use log::debug;
    use std::io;
    use std::path::PathBuf;
    use winreg::{RegKey, enums::HKEY_CLASSES_ROOT};

    fn get_osu_command() -> io::Result<String> {
        RegKey::predef(HKEY_CLASSES_ROOT)
            .open_subkey("osu\\shell\\open\\command")?
            .get_value("")
    }

    fn get_osu_path() -> Option<String> {
        match get_osu_command() {
            Ok(s) => {
                let s = s.trim();
                debug!("Found in registry: {}", s);
                match s.chars().next() {
                    Some('"') => {
                        s[1..].find('"').map(|p| s[1..p + 1].to_owned())
                    },
                    _ => match s.find(' ') {
                        Some(p) => Some(s[0..p].to_owned()),
                        _ => Some(s.to_owned())
                    }
                }
            },
            Err(e) => {
                error!("Error reading the registry: {:?}", e);
                None
            }
        }
    }

    pub fn get_songs_path() -> Option<String> {
        get_osu_path().map(|path| {
            info!("Found osu! installation at {}", path);
            let mut path = PathBuf::from(path);
            path.set_file_name("Songs");
            path.to_str().unwrap().to_owned()
        })
    }
}

#[cfg(windows)]
pub fn find_songs_path() -> Option<String> {
    sys::get_songs_path()
}

#[cfg(not(windows))]
pub fn find_songs_path() -> Option<String> {
    None
}
