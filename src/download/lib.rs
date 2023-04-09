use std::io::Read;

pub fn get_bytes_readable(url: &str) -> Result<impl std::io::Read + std::io::Seek, reqwest::Error> {
    let resp_bytes = reqwest::blocking::get(url)?.bytes()?;
    let readable_bytes = std::io::Cursor::new(resp_bytes);
    Ok(readable_bytes)
}

pub fn unzip_bytes(readable_bytes: impl Read + std::io::Seek) -> zip::result::ZipResult<String> {
    let mut archive = zip::ZipArchive::new(readable_bytes)?;
    let mut file = archive.by_index(0).unwrap();
    let mut contents = String::new();
    std::io::Read::read_to_string(&mut file, &mut contents)?;
    Ok(contents)
}

/// Store the contents of a file in a directory with a timestamped filename ending in .txt.
pub fn store_contents(
    contents: String,
    f_name: &str,
    pwd_dst_dir: &str,
) -> Result<(), std::io::Error> {
    let mut path = std::env::current_dir()?;
    path.push(pwd_dst_dir);

    let dt: chrono::DateTime<chrono::Local> = chrono::Local::now();
    let time_str = dt.format("%Y-%m-%dT%H-%M").to_string();
    path.push(f_name.to_string() + "-" + &time_str + ".txt");

    let mut output_file = std::fs::OpenOptions::new()
        .create(true)
        .write(true)
        .append(true)
        .open(path)?;
    log::debug!(
        "First 100 characters of 'contents': {}",
        contents[0..100].to_string()
    );
    use std::io::prelude::*;

    for line in contents.lines() {
        writeln!(output_file, "{line}")?;
    }
    Ok(())
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_store_contents() {
        let expect_contents = "contents of test_store_contents".to_string();
        let expect_f_name = "test_store_contents";
        let expect_pwd_dst_dir = "managed-files\\prices\\";
        store_contents(expect_contents.clone(), expect_f_name, expect_pwd_dst_dir).unwrap();
        // Assert a file with the prefix "test" exists in the folder
        let mut path = std::env::current_dir().unwrap();
        path.push(expect_pwd_dst_dir);
        let mut found = false;

        for entry in path.read_dir().unwrap() {
            let entry = entry.unwrap();
            let path = entry.path();
            if path.is_file() {
                if path
                    .file_name()
                    .unwrap()
                    .to_str()
                    .unwrap()
                    .contains(expect_f_name)
                {
                    found = true;
                    // Assert the file contains the contents
                    {
                        // Scoped borrow
                        let mut file = std::fs::File::open(&path).unwrap();
                        let mut file_contents = String::new();
                        file.read_to_string(&mut file_contents).unwrap();
                        assert!(file_contents.contains(&expect_contents));
                    }
                    // Delete the file
                    std::fs::remove_file(path).unwrap();
                }
            }
        }
        assert!(found);
    }

    // Doesn't contain any assertions, just prints the results
    // For experimenting and examining folders/files
    #[test]
    #[ignore]
    fn test_first_file_match_from_dir() {
        let price_res = first_file_match_from_dir(
            crate::PRICE_LIST_FNAME,
            &dirs::download_dir().unwrap(),
            Some(1000),
        );
        let card_res = first_file_match_from_dir(
            crate::CARD_DEFINITIONS_FNAME,
            &dirs::download_dir().unwrap(),
            None,
        );
        println!("Price list: {:?}", price_res);
        println!("Card defs list: {:?}", card_res);
    }
}
