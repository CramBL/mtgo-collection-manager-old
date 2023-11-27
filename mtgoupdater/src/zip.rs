use std::marker::PhantomData;
use std::{
    fs,
    path::{Path, PathBuf},
};

/// Markers for the state of an archive
struct UnArchived;
struct Archived;
struct Archive<State = UnArchived> {
    location: PathBuf,
    files: Vec<PathBuf>,
    _state: PhantomData<State>,
}

impl Archive<UnArchived> {
    /// Instantiates a new `Archive`, the archive is not created until `archive` is called, which stores the archive in the location specified by `location`
    pub fn new(location: impl AsRef<Path>) -> Self {
        Self {
            location: location.as_ref().to_path_buf(),
            files: Vec::new(),
            _state: PhantomData,
        }
    }

    pub fn add_file(&mut self, file: PathBuf) {
        self.files.push(file);
    }

    pub fn archive(self) -> Result<Archive<Archived>, std::io::Error> {
        let file = fs::File::create(&self.location)?;
        let mut zip = zip::ZipWriter::new(file);

        let options =
            zip::write::FileOptions::default().compression_method(zip::CompressionMethod::BZIP2);

        for file in &self.files {
            let filename: String = file
                .file_name()
                .unwrap_or_else(|| {
                    panic!(
                        "Failed to get filename from path: {}",
                        file.to_string_lossy()
                    )
                })
                .to_str()
                .unwrap_or_else(|| {
                    panic!(
                        "Failed to convert filename to string: {}",
                        file.to_string_lossy()
                    )
                })
                .to_owned();
            zip.start_file(filename, options)?;
            let mut f = fs::File::open(file)?;
            std::io::copy(&mut f, &mut zip)?;
        }

        zip.finish()?;

        Ok(Archive {
            location: self.location,
            files: self.files,
            _state: PhantomData,
        })
    }
}

impl Archive<Archived> {
    pub fn get_files(&self) -> &[PathBuf] {
        &self.files
    }

    pub fn get_location(&self) -> &Path {
        &self.location
    }
}

#[cfg(test)]
mod tests {
    use std::io::Read;

    use super::*;
    use pretty_assertions::assert_eq;
    use temp_dir::TempDir;

    #[test]
    fn test_archive() {
        let file_contents: String = "1234567891011121314".repeat(10);
        let expect_unzipped_size: usize = file_contents.len();
        const EXPECT_ARCHIVE_SIZE: usize = 175;

        // Create a temporary directory
        let temp_dir = TempDir::new().expect("Failed to create temporary directory");

        // Create a file in the temporary directory
        let temp_file = temp_dir.child("test.txt");
        fs::write(&temp_file, &file_contents).expect("Failed to write to temporary file");

        assert_eq!(
            file_contents,
            fs::read_to_string(&temp_file).expect("Failed to read from temporary file")
        );

        // Create an archive
        let mut archive = Archive::new(temp_dir.child("test.zip"));

        // Add the file to the archive
        archive.add_file(temp_file);

        // Archive the file
        let archived = archive.archive().expect("Failed to archive file");

        // Open the archive
        let file = fs::File::open(archived.get_location()).expect("Failed to open archive");

        let mut zip = zip::ZipArchive::new(file).expect("Failed to create ZipArchive");

        // Get the files from the archive
        let mut zipped_file = zip.by_index(0).expect("Failed to get file from archive");

        let mut zipped_file_contents = String::new();

        zipped_file
            .read_to_string(&mut zipped_file_contents)
            .expect("Failed to read file from archive");

        assert_eq!(file_contents, zipped_file_contents);

        // Size of zip
        let metadata = fs::metadata(archived.get_location()).expect("Failed to get metadata");
        assert_eq!(metadata.len(), EXPECT_ARCHIVE_SIZE as u64);
        eprintln!("Size of zip: {}", metadata.len());
        assert_eq!(zipped_file_contents.len(), expect_unzipped_size);
        eprintln!("unzipped size: {}", zipped_file_contents.len());
    }
}
