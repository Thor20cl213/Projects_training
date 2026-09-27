#include "datelib/date.hpp"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace datelib {

InvalidDateException::InvalidDateException(const std::string& message)
    : std::invalid_argument(message)
{
}

std::string_view toString(Month month)
{
    switch (month) {
        case Month::January:   return "January";
        case Month::February:  return "February";
        case Month::March:     return "March";
        case Month::April:     return "April";
        case Month::May:       return "May";
        case Month::June:      return "June";
        case Month::July:      return "July";
        case Month::August:    return "August";
        case Month::September: return "September";
        case Month::October:   return "October";
        case Month::November:  return "November";
        case Month::December:  return "December";
    }
    return "Unknown";
}

std::string_view toString(Weekday weekday)
{
    switch (weekday) {
        case Weekday::Monday:    return "Monday";
        case Weekday::Tuesday:   return "Tuesday";
        case Weekday::Wednesday: return "Wednesday";
        case Weekday::Thursday:  return "Thursday";
        case Weekday::Friday:    return "Friday";
        case Weekday::Saturday:  return "Saturday";
        case Weekday::Sunday:    return "Sunday";
    }
    return "Unknown";
}

namespace {

std::chrono::sys_days makeSysDays(int year, int month, int day)
{
    if (month < 1 || month > 12) {
        throw InvalidDateException(
            "Mois hors limites (1-12) : " + std::to_string(month));
    }

    const std::chrono::year_month_day ymd{
        std::chrono::year(year),
        std::chrono::month(static_cast<unsigned>(month)),
        std::chrono::day(static_cast<unsigned>(day))};

    if (!ymd.ok()) {
        throw InvalidDateException(
            "Date invalide : " + std::to_string(year) + "-" +
            std::to_string(month) + "-" + std::to_string(day));
    }

    return std::chrono::sys_days{ymd};
}

// Ramene "day" au dernier jour valide de "ym" s'il le depasse (ex: 31 dans
// un mois de 30 jours). Factorise la logique commune a addMonths/addYears.
std::chrono::year_month_day clampToValidDay(
    std::chrono::year_month ym, std::chrono::day day) noexcept
{
    using namespace std::chrono;

    const auto lastDayOfMonth =
        year_month_day_last{ym.year(), month_day_last{ym.month()}}.day();

    return year_month_day{ym.year(), ym.month(), std::min(day, lastDayOfMonth)};
}

} // namespace

Date::Date(std::chrono::sys_days days) noexcept
    : days_(days)
{
}

Date::Date(int year, int month, int day)
    : Date(makeSysDays(year, month, day))
{
}

Date::Date(int year, Month month, int day)
    : Date(year, static_cast<int>(month), day)
{
}

Date Date::today()
{
    return Date{std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now())};
}

Date Date::fromIsoString(std::string_view text)
{
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        throw InvalidDateException(
            "Format attendu YYYY-MM-DD, recu : " + std::string(text));
    }

    const auto parseField = [&text](std::string_view field) {
        int value = 0;
        const auto* begin = field.data();
        const auto* end = field.data() + field.size();
        const auto result = std::from_chars(begin, end, value);

        if (result.ec != std::errc{} || result.ptr != end) {
            throw InvalidDateException(
                "Champ numerique invalide dans la date : " + std::string(text));
        }

        return value;
    };

    const int year = parseField(text.substr(0, 4));
    const int month = parseField(text.substr(5, 2));
    const int day = parseField(text.substr(8, 2));

    return Date(year, month, day);
}

bool Date::isLeapYear(int year) noexcept
{
    return std::chrono::year(year).is_leap();
}

int Date::daysInMonth(int year, Month month) noexcept
{
    using namespace std::chrono;

    const auto last = year_month_day_last{
        std::chrono::year(year),
        month_day_last{std::chrono::month(static_cast<unsigned>(month))}};

    return static_cast<unsigned>(last.day());
}

std::chrono::year_month_day Date::toYearMonthDay() const noexcept
{
    return std::chrono::year_month_day{days_};
}

int Date::year() const noexcept
{
    return static_cast<int>(toYearMonthDay().year());
}

Month Date::month() const noexcept
{
    return static_cast<Month>(static_cast<unsigned>(toYearMonthDay().month()));
}

int Date::day() const noexcept
{
    return static_cast<unsigned>(toYearMonthDay().day());
}

Weekday Date::weekday() const noexcept
{
    return static_cast<Weekday>(std::chrono::weekday{days_}.iso_encoding());
}

Date Date::addDays(int count) const noexcept
{
    return Date{days_ + std::chrono::days(count)};
}

Date Date::addMonths(int count) const noexcept
{
    using namespace std::chrono;

    const auto ymd = toYearMonthDay();
    const year_month newYearMonth = (ymd.year() / ymd.month()) + months(count);

    return Date{sys_days{clampToValidDay(newYearMonth, ymd.day())}};
}

Date Date::addYears(int count) const noexcept
{
    using namespace std::chrono;

    const auto ymd = toYearMonthDay();
    const year_month newYearMonth = (ymd.year() + years(count)) / ymd.month();

    return Date{sys_days{clampToValidDay(newYearMonth, ymd.day())}};
}

std::string Date::toIsoString() const
{
    const auto ymd = toYearMonthDay();

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << static_cast<int>(ymd.year()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.month()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.day());

    return oss.str();
}

Date operator+(const Date& date, Date::Days offset) noexcept
{
    return Date{date.days_ + offset};
}

Date operator+(Date::Days offset, const Date& date) noexcept
{
    return date + offset;
}

Date operator-(const Date& date, Date::Days offset) noexcept
{
    return Date{date.days_ - offset};
}

Date::Days operator-(const Date& lhs, const Date& rhs) noexcept
{
    return lhs.days_ - rhs.days_;
}

std::ostream& operator<<(std::ostream& os, const Date& date)
{
    return os << date.toIsoString();
}

} // namespace datelib
