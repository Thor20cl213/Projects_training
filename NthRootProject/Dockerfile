# Image de base avec Debian + toutes les libs système classiques.
# "bookworm-slim" = Debian 12 allégée (moins de paquets inutiles que l'image complète).
FROM debian:bookworm-slim

# Installe le strict nécessaire pour compiler du C++ :
# - build-essential : gcc, g++, make, libc-dev
# - cmake, ninja-build : outils de build
# On nettoie le cache apt juste après (rm -rf /var/lib/apt/lists/*) pour ne pas
# alourdir l'image avec des fichiers qu'on n'utilisera plus jamais.
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Dossier de travail à l'intérieur du conteneur. Tout ce qui suit (COPY, RUN, CMD)
# s'exécute relativement à ce chemin.
WORKDIR /app

# Copie tout le contenu du dossier courant (sur ta machine) vers /app (dans l'image).
# C'est ici que CMakeLists.txt et src/ atterrissent dans le conteneur.
COPY . .

# Configure et build, exactement les 2 commandes que tu as testées en local.
# Elles s'exécutent une seule fois, PENDANT le "docker build" (pas au lancement).
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build

# Commande exécutée quand tu fais "docker run" (pas pendant le build).
# C'est la 3ème étape : le run.
CMD ["./build/NthRootProject"]
