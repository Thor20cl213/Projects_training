# Version minimale de CMake requise. 3.20 suffit largement pour ce qu'on fait.
cmake_minimum_required(VERSION 3.20)

# Nom du projet + langage. C'est ce qui initialise les compilateurs détectés.
project(DateLibrary LANGUAGES CXX)

# Standard C++ utilisé. "REQUIRED" = échoue si le compilateur ne le supporte pas.
# "EXTENSIONS OFF" = pas d'extensions GNU (-std=gnu++20), on reste en C++ standard pur.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Contrairement aux autres projets du repo, celui-ci est une vraie librairie :
# une cible "datelib" (le code réutilisable) séparée de la cible "DateLibraryDemo"
# (un simple exécutable qui la consomme). C'est ce qui permet de distinguer
# l'API publique (le header sous include/) de l'implémentation (src/date.cpp).
add_library(datelib STATIC src/date.cpp)

# PUBLIC = quiconque link "datelib" hérite automatiquement de ce chemin d'include.
# C'est la façon standard d'exposer l'API d'une lib sans que ses consommateurs
# aient à connaître son organisation interne de dossiers.
target_include_directories(datelib PUBLIC include)

target_compile_options(datelib PRIVATE -Wall -Wextra -Wpedantic)

# L'exécutable de démonstration : uniquement du code consommateur de l'API,
# aucun accès aux détails internes de datelib.
add_executable(DateLibraryDemo src/main.cpp)

target_link_libraries(DateLibraryDemo PRIVATE datelib)

target_compile_options(DateLibraryDemo PRIVATE -Wall -Wextra -Wpedantic)
