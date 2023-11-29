use chrono::{DateTime, NaiveDateTime, ParseResult, Utc};

/// Parses a date string in the format of `YYYY-MM-DDTHHMMSSZ` into a `DateTime<Utc>`
///
/// # Arguments
///
/// * `date` - The date string to parse
///
/// # Errors
///
/// Returns a `ParseError` if the date string cannot be parsed
///
/// # Examples
///
/// ```
/// # use chrono::{Timelike, Datelike};
/// use mtgoupdater::date::parse_naive_date;
///
/// let date = parse_naive_date("2023-11-06T083944Z").unwrap();
/// assert_eq!(date.year(), 2023);
/// assert_eq!(date.month(), 11);
/// assert_eq!(date.day(), 6);
/// assert_eq!(date.hour(), 8);
/// assert_eq!(date.minute(), 39);
/// assert_eq!(date.second(), 44);
/// assert_eq!(date.nanosecond(), 0);
/// ```
pub fn parse_naive_date(date: &str) -> ParseResult<DateTime<Utc>> {
    match NaiveDateTime::parse_from_str(date, "%Y-%m-%dT%H%M%SZ") {
        Ok(datetime) => Ok(datetime.and_utc()),
        Err(e) => Err(e),
    }
}
