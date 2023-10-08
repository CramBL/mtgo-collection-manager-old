pub struct ColumnStyle {
    pub name: &'static str,
    pub width: i32,
    pub idx: i32,
}

impl ColumnStyle {
    pub const fn new(idx: i32, name: &'static str, width: i32) -> Self {
        Self { idx, name, width }
    }
}
