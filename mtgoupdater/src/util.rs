use std::{
    ffi::OsStr,
    process::{Command, Output, Stdio},
};

#[cfg(target_os = "windows")]
use std::os::windows::process::CommandExt;
#[cfg(target_os = "windows")]
const CREATE_NO_WINDOW: u32 = 0x08000000;

pub(super) fn run_with_args<'s, I, S>(bin: S, args: I) -> Result<Output, std::io::Error>
where
    I: IntoIterator<Item = &'s str>,
    S: AsRef<OsStr>,
{
    let mut cmd = Command::new(bin);

    #[cfg(target_os = "windows")]
    cmd.creation_flags(CREATE_NO_WINDOW);

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

    #[cfg(target_os = "windows")]
    cmd.creation_flags(CREATE_NO_WINDOW);

    cmd.stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .stdin(Stdio::piped())
        .arg(arg);

    cmd.output()
}
