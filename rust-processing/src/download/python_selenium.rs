//! Manual download of the target list using python and selenium

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
