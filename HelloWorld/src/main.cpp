#include <cstdlib>
#include <iostream>
#include <utility>  // std::to_underlying (C++23)

enum class Status : int { Ok = 0, HelloWorld = 42 };

int main() {
  // std::to_underlying : nouveauté C++23, évite le vieux static_cast<int>(enum)
  // pour récupérer la valeur sous-jacente d'un enum class.
  std::cout << "Hello depuis le conteneur (ou ta machine) !\n";
  std::cout << "Status code (C++23 std::to_underlying): "
            << std::to_underlying(Status::HelloWorld) << '\n';
  return EXIT_SUCCESS;
}
