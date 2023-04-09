pub fn run_download_script(script_name: &str) {
    let mut path = std::env::current_dir().unwrap();
    path.push(script_name);
    let output = if cfg!(target_os = "windows") {
        std::process::Command::new("cmd")
            .arg("/C")
            .arg("python3.10.exe ".to_string() + path.to_str().unwrap())
            .output()
            .expect("failed to execute process")
    } else {
        std::process::Command::new("sh")
            .arg("-c")
            .arg("python3.10 ".to_string() + path.to_str().unwrap())
            .output()
            .expect("failed to execute process")
    };

    let terminal_response = output.stdout;
    log::info!(
        "Response from command: {}",
        String::from_utf8(terminal_response).unwrap()
    );
}

pub fn first_file_match_from_dir(
    f_name: &str,
    path: &std::path::PathBuf,
    max_file_age: Option<u64>,
) -> Option<std::path::PathBuf> {
    let mut target_lists: Vec<std::path::PathBuf> = Vec::new();

    for entry in path.read_dir().unwrap() {
        let dir_entry = entry.unwrap();

        let metadata = std::fs::metadata(&dir_entry.path()).unwrap();
        let last_modified = metadata.modified().unwrap().elapsed().unwrap().as_secs();
        if metadata.is_file() {
            if let Some(max_file_age) = max_file_age {
                if last_modified > max_file_age {
                    continue;
                }
            }
            log::debug!(
                "Name: {}, Path:{}",
                dir_entry.path().file_name().unwrap().to_str().unwrap(),
                dir_entry.path().display()
            );

            if dir_entry
                .file_name()
                .to_owned()
                .to_str()
                .unwrap()
                .contains(f_name)
            {
                target_lists.push(dir_entry.path());
            }
        }
    }
    if target_lists.len() > 0 {
        return Some(target_lists[0].clone());
    } else {
        log::warn!("No target list found");
        return None;
    }
}

// Opens a zip-file, extracts the first file in the archive, stores it in the specified directory with a timestamp as the suffix of the filename
pub fn extract_and_store(path_to_zip: std::path::PathBuf, f_name: &str, pwd_dst_dir: &str) {
    let file = std::fs::File::open(path_to_zip).unwrap();

    let mut archive = zip::ZipArchive::new(file).unwrap();
    let mut file = archive.by_index(0).unwrap();
    assert!(file.name().contains(f_name));

    let mut contents = String::new();
    file.read_to_string(&mut contents).unwrap();
    let mut path = std::env::current_dir().unwrap();
    path.push(pwd_dst_dir);

    let dt: chrono::DateTime<chrono::Local> = chrono::Local::now();
    let time_str = dt.format("%Y-%m-%dT%H-%M").to_string();
    path.push(f_name.to_string() + "-" + &time_str + ".txt");

    let mut output_file = std::fs::OpenOptions::new()
        .create(true)
        .write(true)
        .append(true)
        .open(path)
        .unwrap();
    log::debug!("Contents: {}", contents[0..100].to_string());
    use std::io::prelude::*;

    for line in contents.lines() {
        writeln!(output_file, "{}", line).unwrap();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // Doesn't contain any assertions, just prints the results
    // For experimenting and examining folders/files
    #[test]
    fn test_first_file_match_from_dir() {
        let price_res = first_file_match_from_dir(
            crate::PRICE_LIST_FNAME,
            &dirs::download_dir().unwrap(),
            Some(1000),
        );
        let card_res = first_file_match_from_dir(
            crate::CARD_DEFINITIONS_FNAME,
            &dirs::download_dir().unwrap(),
            Some(1000),
        );
        println!("Price list: {:?}", price_res);
        println!("Card defs list: {:?}", card_res);
    }
}
