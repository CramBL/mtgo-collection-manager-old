use std::sync::{Arc, Mutex};

/// Represents a column with a value description in the collection table
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

/// Represents the most recent sort state of a column
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

/// Represents the sort order of a column
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Ordering {
    Ascending,
    Descending,
}

impl SortedBy {
    /// Returns true if the column is sorted by descending order
    pub fn is_descending(&self) -> bool {
        matches!(self, SortedBy::Name(d)
            | SortedBy::Quantity(d)
            | SortedBy::Foil(d)
            | SortedBy::Goatbots(d)
            | SortedBy::Scryfall(d)
            | SortedBy::Set(d)
            | SortedBy::Rarity(d) if *d == Ordering::Descending)
    }

    /// Returns true if the column has been sorted by any order at any point
    pub fn is_sorted(&self) -> bool {
        !matches!(self, SortedBy::None)
    }
}

/// Contains the sort state of each column in the collection table
///
/// The values are wrapped in an [Arc] and [Mutex] so that they can be shared between threads
///
/// Only the [SortToggle](super::SortToggle) buttons should have direct access to the values, and they should only to read them.
/// All other access should be done through the [SortStates] methods.
pub struct SortStates {
    /// The sort state of the name column
    pub name: Arc<Mutex<SortedBy>>,
    /// The sort state of the quantity column
    pub quantity: Arc<Mutex<SortedBy>>,
    /// The sort state of the foil column
    pub foil: Arc<Mutex<SortedBy>>,
    /// The sort state of the Goatbots column
    pub goatbots: Arc<Mutex<SortedBy>>,
    /// The sort state of the Cardhoarder column
    pub cardhoarder: Arc<Mutex<SortedBy>>,
    /// The sort state of the set column
    pub set: Arc<Mutex<SortedBy>>,
    /// The sort state of the rarity column
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

    /// Returns the [SortedBy] state of the `name` column
    pub fn name_ord(&self) -> SortedBy {
        *self.name.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `name` column
    pub fn set_name_ord(&mut self, new_ord: SortedBy) {
        *self.name.lock().unwrap() = new_ord;
    }

    /// Returns the [SortedBy] state of the `quantity` column
    pub fn quantity_ord(&self) -> SortedBy {
        *self.quantity.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `quantity` column
    pub fn set_quantity_ord(&mut self, new_ord: SortedBy) {
        *self.quantity.lock().unwrap() = new_ord;
    }

    /// Returns the [SortedBy] state of the `foil` column
    pub fn foil_ord(&self) -> SortedBy {
        *self.foil.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `foil` column
    pub fn set_foil_ord(&mut self, new_ord: SortedBy) {
        *self.foil.lock().unwrap() = new_ord;
    }

    /// Returns the [SortedBy] state of the `goatbots` column
    pub fn goatbots_ord(&self) -> SortedBy {
        *self.goatbots.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `goatbots` column
    pub fn set_goatbots_ord(&mut self, new_ord: SortedBy) {
        *self.goatbots.lock().unwrap() = new_ord;
    }

    /// Returns the [SortedBy] state of the `cardhoarder` column
    pub fn cardhoarder_ord(&self) -> SortedBy {
        *self.cardhoarder.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `cardhoarder` column
    pub fn set_cardhoarder_ord(&mut self, new_ord: SortedBy) {
        *self.cardhoarder.lock().unwrap() = new_ord;
    }

    /// Returns the [SortedBy] state of the `set` column
    pub fn set_ord(&self) -> SortedBy {
        *self.set.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `set` column
    pub fn set_set_ord(&mut self, new_ord: SortedBy) {
        *self.set.lock().unwrap() = new_ord;
    }

    /// Returns the [SortedBy] state of the `rarity` column
    pub fn rarity_ord(&self) -> SortedBy {
        *self.rarity.lock().unwrap()
    }
    /// Sets the [SortedBy] state of the `rarity` column
    pub fn set_rarity_ord(&mut self, new_ord: SortedBy) {
        *self.rarity.lock().unwrap() = new_ord;
    }
}

impl Default for SortStates {
    fn default() -> Self {
        Self::new()
    }
}
