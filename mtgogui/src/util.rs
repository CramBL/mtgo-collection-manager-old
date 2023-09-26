use std::sync::OnceLock;

use fltk::image::PngImage;

// Logo placed left-most in window labels
const MCM_LOGO_RAW: &[u8; 3542] = include_bytes!("../assets/35x35-logo-card-pile.png");
pub static MCM_LOGO: OnceLock<PngImage> = OnceLock::new();
pub fn get_logo() -> PngImage {
    MCM_LOGO
        .get_or_init(|| PngImage::from_data(MCM_LOGO_RAW).unwrap())
        .clone()
}

pub fn center() -> (i32, i32) {
    (
        (fltk::app::screen_size().0 / 2.0) as i32,
        (fltk::app::screen_size().1 / 2.0) as i32,
    )
}

#[cfg(windows)]
pub fn hide_console_window() {
    use winapi::um::wincon::GetConsoleWindow;
    use winapi::um::winuser;

    let window = unsafe { GetConsoleWindow() };
    if !window.is_null() {
        unsafe {
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
            // Hide takes no effect if the window is not active, so we activate it before hiding (minimizing) it.
            // The window being active at the time of calling this varies between launching the app.
            // If the window is not active when attempting to hide it, it will just be displayed behind the app.
            _ = winuser::ShowWindow(window, winuser::SW_SHOWMINIMIZED);
            let _was_visible = winuser::ShowWindow(window, winuser::SW_HIDE);
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablewindow
            winuser::EnableWindow(window, 0);
        }
        println!(
            r"Hi MTGO Collection Manager Windows user. This terminal window runs MTGO Collection Manager, please don't close it
(same result as pressing X in the app). Sorry to inconvenience you with this ugly terminal window, it will be
removed in a future version when another form of inter-process communication is implemented"
        );
    }
}
