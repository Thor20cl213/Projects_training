# Version minimale de CMake requise. 3.20 suffit largement pour ce qu'on fait.
cmake_minimum_required(VERSION 3.20)

# Nom du projet + langage. C'est ce qui initialise les compilateurs détectés.
project(HashCode LANGUAGES CXX)

# Standard C++ utilisé. "REQUIRED" = échoue si le compilateur ne le supporte pas.
# "EXTENSIONS OFF" = pas d'extensions GNU (-std=gnu++20), on reste en C++ standard pur.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# La cible : un exécutable nommé "HashCode", construit à partir de main.cpp.
# C'est la "target-based" approach : tout se rattache à une cible précise,
# jamais de variable globale comme CMAKE_CXX_FLAGS.
add_executable(HashCode src/main.cpp)

# Warnings stricts appliqués UNIQUEMENT à cette cible (PRIVATE = pas propagé
# à qui l'utiliserait comme dépendance, ce qui n'a pas de sens ici de toute façon
# vu qu'un exécutable n'est jamais "linké" par un autre projet).
target_compile_options(HashCode PRIVATE -Wall -Wextra -Wpedantic)
