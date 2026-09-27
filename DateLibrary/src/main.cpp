#include "datelib/date.hpp"

#include <iostream>

using datelib::Date;
using datelib::InvalidDateException;

int main()
{
    // Construction et affichage de base.
    const Date bastilleDay(2024, datelib::Month::July, 14);
    std::cout << "Date : " << bastilleDay
              << " (" << toString(bastilleDay.weekday()) << ")\n";

    // Arithmetique sur les jours, via le type fort Date::Days.
    const Date oneWeekLater = bastilleDay + Date::Days(7);
    std::cout << "Une semaine plus tard : " << oneWeekLater << '\n';

    // Difference entre deux dates -> Date::Days, pas un int brut.
    const Date::Days gap = oneWeekLater - bastilleDay;
    std::cout << "Ecart : " << gap.count() << " jour(s)\n";

    // addMonths applique une troncature explicite quand le jour cible
    // n'existe pas dans le mois d'arrivee.
    const Date endOfJanuary(2024, 1, 31);
    std::cout << "\n" << endOfJanuary << " + 1 mois = "
              << endOfJanuary.addMonths(1)
              << " (2024 est bissextile : "
              << (Date::isLeapYear(2024) ? "oui" : "non") << ")\n";

    // addYears applique la meme regle sur le 29 fevrier.
    const Date leapDay(2024, 2, 29);
    std::cout << leapDay << " + 1 an = " << leapDay.addYears(1) << '\n';

    // Comparaisons via l'operator<=> defaulte.
    std::cout << "\n" << bastilleDay << " < " << oneWeekLater << " : "
              << std::boolalpha << (bastilleDay < oneWeekLater) << '\n';

    // Parsing ISO 8601 et gestion d'erreur via l'exception dediee.
    const Date parsed = Date::fromIsoString("2000-01-01");
    std::cout << "\nDate parsee : " << parsed
              << " (" << toString(parsed.month()) << " "
              << parsed.day() << ", " << parsed.year() << ")\n";

    try {
        Date::fromIsoString("2000-02-30");
    } catch (const InvalidDateException& e) {
        std::cout << "Exception attendue capturee : " << e.what() << '\n';
    }

    std::cout << "\nToday : " << Date::today() << '\n';

    return 0;
}
