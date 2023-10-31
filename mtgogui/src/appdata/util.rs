use super::APP_DATA_DIR;
use std::io;
use std::path::PathBuf;

/// Get the path to the appdata directory
///
/// # Errors
///
/// * If the path to the appdata directory cannot be determined
/// * If the path to the appdata directory doesn't exist
pub fn appdata_path() -> io::Result<PathBuf> {
    let mut appdata_dir = std::env::current_exe()?;
    log::info!("Path to executable: {appdata_dir:?}");
    appdata_dir.pop();
    if cfg!(windows) {
        appdata_dir.push(format!(r#"{APP_DATA_DIR}\"#));
    } else {
        appdata_dir.push(format!(r#"{APP_DATA_DIR}/"#));
    }
    log::info!("Path to appdata dir: {appdata_dir:?}");

    if !appdata_dir.exists() {
        log::info!("App data path doesn't exist! - {appdata_dir:?}");
        return Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            format!("App data path {APP_DATA_DIR} doesn't exist!"),
        ));
    }

    Ok(appdata_dir)
}
