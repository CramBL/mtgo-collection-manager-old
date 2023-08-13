use std::process::Command;

pub fn download_goatbots_price_history() -> Result<std::process::Output, Box<dyn std::error::Error>>
{
    let go_exec_out = Command::new("mtgogetter")
        .arg("download")
        .arg("gph")
        .output()?;

    Ok(go_exec_out)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_call_mtgogetter_download_price_history() {
        let test_out = download_goatbots_price_history();
        match test_out {
            Ok(output) => println!(
                "Status: {status}, stdout: {stdout:?}, stderr {stderr:?}",
                status = output.status,
                stdout = output.stdout,
                stderr = output.stderr
            ),
            Err(e) => assert!(false, "Unexpected error: {e}"),
        }
    }
}
