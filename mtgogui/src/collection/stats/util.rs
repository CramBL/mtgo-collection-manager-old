#[derive(Debug, Clone, Default, Copy)]
pub struct UniqueTotal {
    unique: usize,
    total: usize,
}

impl UniqueTotal {
    pub fn new(unique: usize, total: usize) -> Self {
        Self { unique, total }
    }

    pub fn unique(&self) -> usize {
        self.unique
    }

    pub fn total(&self) -> usize {
        self.total
    }
}
