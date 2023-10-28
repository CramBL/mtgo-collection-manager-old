use flexi_logger::{
    Cleanup, Criterion, Duplicate, FileSpec, Logger, LoggerHandle, Naming, WriteMode,
};

pub fn center() -> (i32, i32) {
    (
        (fltk::app::screen_size().0 / 2.0) as i32,
        (fltk::app::screen_size().1 / 2.0) as i32,
    )
}

pub fn first_file_match_from_dir(
    f_name: &str,
    path: &std::path::Path,
    max_file_age_secs: Option<u64>,
) -> Option<std::path::PathBuf> {
    for entry in path.read_dir().unwrap() {
        let dir_entry = entry.unwrap();

        let metadata = std::fs::metadata(&dir_entry.path()).unwrap();
        let last_modified = metadata.modified().unwrap().elapsed().unwrap().as_secs();
        if metadata.is_file() {
            if let Some(max_file_age) = max_file_age_secs {
                if last_modified > max_file_age {
                    continue;
                }
            }

            if dir_entry.file_name().to_string_lossy().contains(f_name) {
                return Some(dir_entry.path());
            }
        }
    }

    None
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
