use std::{
    io::Error,
    path::{Path, PathBuf},
    time::{Duration, Instant, SystemTime},
};

use flexi_logger::{
    Cleanup, Criterion, Duplicate, FileSpec, Logger, LoggerHandle, Naming, WriteMode,
};

pub fn center() -> (i32, i32) {
    (
        (fltk::app::screen_size().0 / 2.0) as i32,
        (fltk::app::screen_size().1 / 2.0) as i32,
    )
}

/// Find the first file in the given directory that contains the given string
///
/// # Arguments
///
/// * `f_name` - The string to search for in the file names
/// * `path` - The path to the directory to search in
/// * `max_file_age_secs` - If set, only files younger than this many seconds will be considered
///
/// # Returns
///
/// The path to the first file that contains the given string, or None if no such file was found
///
/// # Errors
///
/// * If the given path is not a directory
/// * If the given path cannot be read
/// * If the metadata of a file in the given directory cannot be read (permissions)
/// * If the last modified time of a file in the given directory cannot be read
/// * If the last modified time of a file in the given directory is in the future (very unlikely, but possible because of system clock drift)
pub fn first_file_match_from_dir(
    f_name: &str,
    path: &Path,
    max_file_age_secs: Option<u64>,
) -> Result<Option<PathBuf>, Error> {
    for entry in path.read_dir()? {
        let dir_entry = entry?;

        let metadata = std::fs::metadata(&dir_entry.path())?;
        let last_modified = metadata
            .modified()?
            .elapsed()
            .unwrap_or_else(|_| Duration::from_nanos(1)) // If the file was modified in the future, pretend it was modified 1 nanosecond ago
            .as_secs();

        if metadata.is_file() {
            if let Some(max_file_age) = max_file_age_secs {
                if last_modified > max_file_age {
                    continue;
                }
            }

            if dir_entry.file_name().to_string_lossy().contains(f_name) {
                return Ok(Some(dir_entry.path()));
            }
        }
    }

    Ok(None)
}

/// Setup the logger
///
/// Returns a handle to the logger which has to stay alive for the duration of the program
pub fn setup_logger() -> LoggerHandle {
    // Get the path to the MTGO GUI executable
    let mut appdata_dir = std::env::current_exe().unwrap();
    // Remove the executable from the path, then the path points at the directory
    appdata_dir.pop();
    // Add the appdata directory to the path
    appdata_dir.push("appdata");
    // Add the log_files directory to the path
    appdata_dir.push("log_files");

    // 5 MiB
    const MAX_LOG_FILE_SIZE: u64 = 5 * 1024 * 1024;

    Logger::try_with_str("info")
        .expect("Failed setting up logger")
        // Log to a file in the appdata directory
        .log_to_file(
            FileSpec::default()
                .directory(appdata_dir)
                .basename("mcm_log"),
        )
        .rotate(
            // If the program runs long enough:
            // - create a new file every day
            Criterion::Size(MAX_LOG_FILE_SIZE),
            // - let the rotated files have a timestamp in their name
            Naming::Timestamps,
            // - keep at most 7 log files (7 + current log file)
            Cleanup::KeepLogFiles(7),
        )
        // - write all messages with `info` or higher verbosity to stderr
        .duplicate_to_stderr(Duplicate::Info)
        // Configure for asynchronous logging
        .write_mode(WriteMode::Async)
        .start()
        .expect("Failed to initialize logger")
}

#[cfg(test)]
mod tests {
    use std::path::PathBuf;

    use super::*;

    #[test]
    fn test_find_first_match_cargolock() {
        let cwd = std::env::current_dir().unwrap();
        let first_match = first_file_match_from_dir("Cargo.lock", &cwd, None);

        assert_eq!(
            PathBuf::from("Cargo.lock"),
            first_match.unwrap().file_name().unwrap()
        );
    }

    #[test]
    fn test_find_first_match_cargotoml() {
        let cwd = std::env::current_dir().unwrap();
        let first_match = first_file_match_from_dir("Cargo.toml", &cwd, None);

        assert_eq!(
            PathBuf::from("Cargo.toml"),
            first_match.unwrap().file_name().unwrap()
        );
    }

    #[test]
    fn test_find_first_match_cargo() {
        // Searching for "Cargo" can find either Cargo.lock or Cargo.toml

        let cwd = std::env::current_dir().unwrap();
        let first_match = first_file_match_from_dir("Cargo", &cwd, None);

        let path = first_match.unwrap();
        let name = path.file_name().unwrap();

        if name != "Cargo.lock" && name != "Cargo.toml" {
            panic!("Did not get Cargo.lock or Cargo.toml, got: {name:?}")
        }
    }
}
