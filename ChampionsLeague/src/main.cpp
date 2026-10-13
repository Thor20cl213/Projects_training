#include <algorithm>
#include <array>
#include <functional>
#include <print>
#include <random>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

constexpr int NB_EQUIPES = 36;
constexpr int NB_POTS = 4;
constexpr int TAILLE_POT = NB_EQUIPES / NB_POTS;
constexpr int NB_MATCHS = 8;
constexpr int MAX_POINTS = 3 * NB_MATCHS;
constexpr int PLACE_TOP8 = 8;
constexpr int PLACE_TOP24 = 24;

constexpr double P_NUL = 0.20;
constexpr double P_VICTOIRE_DOMICILE = (1.0 - P_NUL) / 2.0;

constexpr long long N = 250'000;
constexpr int NB_THREADS = 8;

using Histogramme = std::array<long long, MAX_POINTS + 1>;
using Calendrier = std::vector<std::pair<int, int>>;

struct Stats
{
  std::array<Histogramme, NB_EQUIPES> points_par_place{};
  Histogramme equipes{};
  Histogramme top8{};
  Histogramme top24{};
};

// Format réel : 4 pots de 9, chaque équipe affronte 2 adversaires de chaque pot (le sien compris).
Calendrier calendrier()
{
  Calendrier matchs;
  for(int a = 0; a<NB_POTS; a++)
  {
    for(int b = a; b<NB_POTS; b++)
    {
      for(int i = 0; i<TAILLE_POT; i++)
      {
        int equipe = a*TAILLE_POT + i;
        matchs.emplace_back(equipe, b*TAILLE_POT + (i+1)%TAILLE_POT);
        if(a != b)
        {
          matchs.emplace_back(equipe, b*TAILLE_POT + i);
        }
      }
    }
  }
  return matchs;
}

void task(const Calendrier& matchs, Stats& stats)
{
  std::mt19937_64 seed{std::random_device{}()};
  std::uniform_real_distribution<double> dist{0.0,1.0};
  std::array<int, NB_EQUIPES> points;

  for(long long s = 0; s<N; s++)
  {
    points.fill(0);
    for(auto [domicile, exterieur] : matchs)
    {
      double u = dist(seed);
      if(u < P_NUL)
      {
        points[domicile] += 1;
        points[exterieur] += 1;
      }
      else if(u < P_NUL + P_VICTOIRE_DOMICILE)
      {
        points[domicile] += 3;
      }
      else
      {
        points[exterieur] += 3;
      }
    }

    std::ranges::sort(points, std::greater{});

    // Les équipes à égalité de points occupent les places [debut, fin) : elles se partagent
    // équitablement les places qualificatives du bloc (départage aléatoire, pas de buts simulés).
    for(int debut = 0; debut<NB_EQUIPES;)
    {
      int pts = points[debut];
      int fin = debut;
      while(fin<NB_EQUIPES && points[fin] == pts)
      {
        stats.points_par_place[fin][pts]++;
        fin++;
      }
      stats.equipes[pts] += fin - debut;
      stats.top8[pts] += std::max(0, std::min(fin, PLACE_TOP8) - debut);
      stats.top24[pts] += std::max(0, std::min(fin, PLACE_TOP24) - debut);
      debut = fin;
    }
  }
}

int mode(const Histogramme& h)
{
  return static_cast<int>(std::ranges::max_element(h) - h.begin());
}

double moyenne(const Histogramme& h, long long total)
{
  double somme = 0.0;
  for(int pts = 0; pts<=MAX_POINTS; pts++)
  {
    somme += static_cast<double>(pts) * h[pts];
  }
  return somme / total;
}

void afficher_palier(std::string_view nom, int place, const Histogramme& h, long long total)
{
  std::println("\nPalier {} (points du {}e) :", nom, place);
  for(int pts = MAX_POINTS; pts>=0; pts--)
  {
    double p = 100.0 * h[pts] / total;
    if(p >= 0.1)
    {
      std::println("  {:2} pts : {:5.1f} %", pts, p);
    }
  }
  std::println("  => maximum de vraisemblance : {} pts (moyenne {:.2f})", mode(h), moyenne(h, total));
}

int main() {

  const Calendrier matchs = calendrier();
  std::vector<Stats> stats_threads(NB_THREADS);

  {
    std::vector<std::jthread> threads;
    for(int i = 0;i<NB_THREADS;i++)
    {
      threads.emplace_back(&task, std::cref(matchs), std::ref(stats_threads[i]));
    }
  }

  Stats stats;
  for(const Stats& s : stats_threads)
  {
    for(int pts = 0; pts<=MAX_POINTS; pts++)
    {
      stats.equipes[pts] += s.equipes[pts];
      stats.top8[pts] += s.top8[pts];
      stats.top24[pts] += s.top24[pts];
      for(int place = 0; place<NB_EQUIPES; place++)
      {
        stats.points_par_place[place][pts] += s.points_par_place[place][pts];
      }
    }
  }

  const long long total = N*NB_THREADS;

  std::println("Phase de ligue simulée :{} fois ({} équipes, {} matchs)", total, NB_EQUIPES, matchs.size());
  std::println("Probabilités par match : nul {:.0f} %, victoire {:.0f} % de chaque côté",
               100.0*P_NUL, 100.0*P_VICTOIRE_DOMICILE);

  afficher_palier("top 8", PLACE_TOP8, stats.points_par_place[PLACE_TOP8-1], total);
  afficher_palier("top 24", PLACE_TOP24, stats.points_par_place[PLACE_TOP24-1], total);

  std::println("\nProbabilité de qualification selon le total de points :");
  std::println("  pts |   top 8 |  top 24");
  for(int pts = MAX_POINTS; pts>=0; pts--)
  {
    if(stats.equipes[pts] > 0)
    {
      std::println("  {:3} | {:5.1f} % | {:5.1f} %", pts,
                   100.0 * stats.top8[pts] / stats.equipes[pts],
                   100.0 * stats.top24[pts] / stats.equipes[pts]);
    }
  }

  std::println("\nPoints par place finale :");
  std::println("  place | plus probable | moyenne");
  for(int place = 0; place<NB_EQUIPES; place++)
  {
    const Histogramme& h = stats.points_par_place[place];
    std::println("  {:5} | {:13} | {:7.2f}", place+1, mode(h), moyenne(h, total));
  }

  return 0;

}
