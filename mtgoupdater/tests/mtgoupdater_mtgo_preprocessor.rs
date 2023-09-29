use mtgoupdater::internal_only;

#[test]
fn test_call_mtgo_preprocessor() {
    internal_only::dev_try_init_mtgoparser_bin();
    // Check the build directory exists
    assert!(
        std::path::Path::new("../mtgoparser/build").exists(),
        "Build directory does not exist, build mtgoparser before running this test"
    );
    // Check the build src mtgo_preprocessor directory exists
    assert!(
        std::path::Path::new("../mtgoparser/build/src/mtgo_preprocessor").exists(),
        "mtgo_preprocessor directory does not exist, build mtgoparser before running this test"
    );
    // Check the mtgo_preprocessor binary exists
    assert!(
        std::path::Path::new(internal_only::DEV_MTGOPARSER_BIN).exists(),
        "mtgo_preprocessor binary ({mtgoparser_bin}) does not exist, build mtgoparser before running this test", mtgoparser_bin = internal_only::DEV_MTGOPARSER_BIN
    );

    let test_out = mtgoupdater::run_mtgo_preprocessor_example();
    match test_out {
        Ok(output) => {
            println!("Status:\n{status}", status = output.status,);
            println!(
                "stdout:\n{stdout}",
                stdout = String::from_utf8_lossy(&output.stdout),
            );
            println!(
                "stderr:\n{stderr}",
                stderr = String::from_utf8_lossy(&output.stderr),
            );
            assert!(
                output.status.success(),
                "Process failed with non-zero exit code: {}",
                output.status.code().unwrap_or(123)
            );
        }
        Err(e) => panic!("Unexpected error: {e}"),
    }
}

#[test]
fn test_call_mtgo_preprocessor_json_example() {
    internal_only::dev_try_init_mtgoparser_bin();
    match mtgoupdater::run_mtgo_preprocessor_example() {
        Ok(output) => {
            println!("Status:\n{status}", status = output.status,);

            println!(
                "stdout:\n{stdout}",
                stdout = String::from_utf8_lossy(&output.stdout),
            );
            println!(
                "stderr:\n{stderr}",
                stderr = String::from_utf8_lossy(&output.stderr),
            );
            assert!(
                output.status.success(),
                "Process failed with non-zero exit code: {}",
                output.status.code().unwrap_or(123)
            );
        }
        Err(e) => panic!("Unexpected error: {e}"),
    }
}

#[test]
fn test_call_mtgo_preprocessor_gui_example() {
    internal_only::dev_try_init_mtgoparser_bin();
    match mtgoupdater::internal_only::run_mtgo_preprocessor_gui_example() {
        Ok(output) => {
            println!("Status:\n{status}", status = output.status,);

            println!(
                "stdout:\n{stdout}",
                stdout = String::from_utf8_lossy(&output.stdout),
            );
            println!(
                "stderr:\n{stderr}",
                stderr = String::from_utf8_lossy(&output.stderr),
            );
            assert!(
                output.status.success(),
                "Process failed with non-zero exit code: {}",
                output.status.code().unwrap_or(123)
            );
        }
        Err(e) => panic!("Unexpected error: {e}"),
    }
}
