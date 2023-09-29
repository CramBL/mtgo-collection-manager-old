use std::{
    ffi::OsStr,
    os::windows::process::CommandExt,
    process::{Command, Output, Stdio},
};

const CREATE_NO_WINDOW: u32 = 0x08000000;

pub(super) fn run_with_args<I, S>(bin: S, args: I) -> Result<Output, std::io::Error>
where
    I: IntoIterator<Item = S>,
    S: AsRef<OsStr>,
{
    let mut cmd = Command::new(bin);

    if cfg!(target_os = "windows") {
        cmd.creation_flags(CREATE_NO_WINDOW);
    }

    cmd.stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .stdin(Stdio::piped())
        .args(args);

    cmd.output()
}

pub(super) fn run_with_arg<S>(bin: S, arg: S) -> Result<Output, std::io::Error>
where
    S: AsRef<OsStr>,
{
    let mut cmd = Command::new(bin);

    if cfg!(target_os = "windows") {
        cmd.creation_flags(CREATE_NO_WINDOW);
    }

    cmd.stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .stdin(Stdio::piped())
        .arg(arg);

    cmd.output()
}
