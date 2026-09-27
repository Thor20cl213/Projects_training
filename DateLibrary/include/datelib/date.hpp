#pragma once

#include <chrono>
#include <compare>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <string_view>

namespace datelib {

// Exception dediee plutot qu'un std::invalid_argument generique :
// permet aux appelants de distinguer "date invalide" d'une autre erreur
// d'argument, et documente l'intention dans le type lui-meme.
class InvalidDateException : public std::invalid_argument {
public:
    explicit InvalidDateException(const std::string& message);
};

enum class Month : unsigned {
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};

// Encodage ISO 8601 : Monday = 1 ... Sunday = 7.
enum class Weekday : unsigned {
    Monday = 1,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

std::string_view toString(Month month);
std::string_view toString(Weekday weekday);

// Type de date immuable (comme std::chrono::sys_days) : toute operation qui
// "modifie" une date retourne une nouvelle instance plutot que de muter
// l'objet courant. Ca evite toute une classe de bugs d'alias/etat partage,
// au prix d'une petite copie (un Date ne pese qu'un entier de jours).
class Date {
public:
    // Duree en jours, exposee via le type fort de <chrono> plutot qu'un
    // simple int : impossible de confondre un nombre de jours avec une
    // annee ou un jour du mois a la compilation.
    using Days = std::chrono::days;

    // Leve InvalidDateException si (year, month, day) ne forme pas une date
    // valide du calendrier gregorien (ex: 31 fevrier, mois hors [1, 12]...).
    Date(int year, int month, int day);
    Date(int year, Month month, int day);

    static Date today();

    // Parse le format ISO 8601 strict "YYYY-MM-DD".
    // Leve InvalidDateException si le format ou la date est invalide.
    static Date fromIsoString(std::string_view text);

    static bool isLeapYear(int year) noexcept;
    static int daysInMonth(int year, Month month) noexcept;

    int year() const noexcept;
    Month month() const noexcept;
    int day() const noexcept;
    Weekday weekday() const noexcept;

    Date addDays(int count) const noexcept;

    // Ajoute des mois calendaires. Si le jour courant n'existe pas dans le
    // mois cible (ex: 31 janvier + 1 mois), il est ramene au dernier jour
    // valide de ce mois (-> 28 ou 29 fevrier). C'est le comportement le plus
    // courant, mais il vaut la peine d'etre documente explicitement : une
    // addition de mois n'a pas de definition mathematique unique.
    Date addMonths(int count) const noexcept;

    // Meme regle de troncature que addMonths, appliquee a l'annee (utile
    // pour le 29 fevrier d'une annee bissextile).
    Date addYears(int count) const noexcept;

    std::string toIsoString() const;

    friend auto operator<=>(const Date&, const Date&) = default;

    friend Date operator+(const Date& date, Days offset) noexcept;
    friend Date operator+(Days offset, const Date& date) noexcept;
    friend Date operator-(const Date& date, Days offset) noexcept;
    friend Days operator-(const Date& lhs, const Date& rhs) noexcept;

    friend std::ostream& operator<<(std::ostream& os, const Date& date);

private:
    explicit Date(std::chrono::sys_days days) noexcept;

    std::chrono::year_month_day toYearMonthDay() const noexcept;

    std::chrono::sys_days days_;
};

} // namespace datelib
