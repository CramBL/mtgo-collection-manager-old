use std::sync::{Arc, Mutex};

#[derive(Debug, Clone, Copy)]
pub enum Column {
    Name,
    Quantity,
    Foil,
    Goatbots,
    Scryfall,
    Set,
    Rarity,
}

#[derive(Debug, Clone, Copy, PartialEq)]

pub enum SortedBy {
    None,
    Name(Ordering),
    Quantity(Ordering),
    Foil(Ordering),
    Goatbots(Ordering),
    Scryfall(Ordering),
    Set(Ordering),
    Rarity(Ordering),
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Ordering {
    Ascending,
    Descending,
}

impl SortedBy {
    pub fn is_descending(&self) -> bool {
        match self {
            SortedBy::Name(d)
            | SortedBy::Quantity(d)
            | SortedBy::Foil(d)
            | SortedBy::Goatbots(d)
            | SortedBy::Scryfall(d)
            | SortedBy::Set(d)
            | SortedBy::Rarity(d)
                if *d == Ordering::Descending =>
            {
                true
            }
            _ => false,
        }
    }

    pub fn is_sorted(&self) -> bool {
        match self {
            SortedBy::None => false,
            _ => true,
        }
    }
}

pub struct SortStates {
    pub name: Arc<Mutex<SortedBy>>,
    pub quantity: Arc<Mutex<SortedBy>>,
    pub foil: Arc<Mutex<SortedBy>>,
    pub goatbots: Arc<Mutex<SortedBy>>,
    pub cardhoarder: Arc<Mutex<SortedBy>>,
    pub set: Arc<Mutex<SortedBy>>,
    pub rarity: Arc<Mutex<SortedBy>>,
}

impl SortStates {
    pub fn new() -> Self {
        // Set all as sorted by ascending as default, as they are not sorted and then the first toggle will sort descending
        Self {
            name: Arc::new(Mutex::new(SortedBy::None)),
            quantity: Arc::new(Mutex::new(SortedBy::None)),
            foil: Arc::new(Mutex::new(SortedBy::None)),
            goatbots: Arc::new(Mutex::new(SortedBy::None)),
            cardhoarder: Arc::new(Mutex::new(SortedBy::None)),
            set: Arc::new(Mutex::new(SortedBy::None)),
            rarity: Arc::new(Mutex::new(SortedBy::None)),
        }
    }

    pub fn name_ord(&self) -> SortedBy {
        *self.name.lock().unwrap()
    }
    pub fn set_name_ord(&mut self, new_ord: SortedBy) {
        *self.name.lock().unwrap() = new_ord;
    }

    pub fn quantity_ord(&self) -> SortedBy {
        *self.quantity.lock().unwrap()
    }
    pub fn set_quantity_ord(&mut self, new_ord: SortedBy) {
        *self.quantity.lock().unwrap() = new_ord;
    }

    pub fn foil_ord(&self) -> SortedBy {
        *self.foil.lock().unwrap()
    }
    pub fn set_foil_ord(&mut self, new_ord: SortedBy) {
        *self.foil.lock().unwrap() = new_ord;
    }

    pub fn goatbots_ord(&self) -> SortedBy {
        *self.goatbots.lock().unwrap()
    }
    pub fn set_goatbots_ord(&mut self, new_ord: SortedBy) {
        *self.goatbots.lock().unwrap() = new_ord;
    }

    pub fn cardhoarder_ord(&self) -> SortedBy {
        *self.cardhoarder.lock().unwrap()
    }
    pub fn set_cardhoarder_ord(&mut self, new_ord: SortedBy) {
        *self.cardhoarder.lock().unwrap() = new_ord;
    }

    pub fn set_ord(&self) -> SortedBy {
        *self.set.lock().unwrap()
    }
    pub fn set_set_ord(&mut self, new_ord: SortedBy) {
        *self.set.lock().unwrap() = new_ord;
    }

    pub fn rarity_ord(&self) -> SortedBy {
        *self.rarity.lock().unwrap()
    }
    pub fn set_rarity_ord(&mut self, new_ord: SortedBy) {
        *self.rarity.lock().unwrap() = new_ord;
    }
}

impl Default for SortStates {
    fn default() -> Self {
        Self::new()
    }
}
