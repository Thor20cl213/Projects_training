#include <random>
#include <cmath>
#include <thread>
#include <print>
#include <atomic>
#include <numbers>

std::atomic<long long> dans_cercle{0};
constexpr long long N = 1'000'000;
constexpr int NB_THREADS = 8;

void task(int num_thread)
{
  std::mt19937_64 seed{static_cast<unsigned long long>(42+num_thread)}; 
  std::uniform_real_distribution<double> dist{0.0,1.0};

  for(long long i = 0; i<N; i++)
  {
    double x = dist(seed);
    double y = dist(seed);
    if(x*x + y*y <= 1.0)
    {
      dans_cercle.fetch_add(1);
    }
  }
}
int main() {
  
  std::thread threads[NB_THREADS];
  for(int i = 0;i<NB_THREADS;i++)
  {
    threads[i]=std::thread(&task,i);
  }

  for(int i = 0;i<NB_THREADS;i++)
  {
    threads[i].join();
  }

  double pi_estime = (4.0 * dans_cercle.load())/ (N*NB_THREADS);

  std::println("Point totaux :{}",N*NB_THREADS);
  std::println("Dans le cercle :{}",dans_cercle.load());
  std::println("Pi estimé :{}",pi_estime);
  std::println("Pi réel :{}",std::numbers::pi);
  std::println("Erreur :{}",std::abs(std::numbers::pi - pi_estime));

  return 0;

}
