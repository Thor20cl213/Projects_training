# Projets_CPP

Collection de petits projets C++23 indépendants, à but pédagogique (apprentissage du langage,
de la concurrence, de CMake, et du debugging bas niveau). Chaque dossier à la racine est un
projet autonome avec sa propre cible CMake — il n'y a pas de code partagé entre eux.

## Structure type d'un projet

```
NomDuProjet/
  CMakeLists.txt   # cmake_minimum_required 3.20, C++23, warnings -Wall -Wextra -Wpedantic
  Dockerfile       # image debian:bookworm-slim, build via cmake+ninja
  .dockerignore
  src/main.cpp     # tout le code tient dans un seul fichier
  build/           # généré, ne pas éditer à la main
```

Toolchain locale : Clang 20.1.8 (`C:/Program Files/clang+llvm-20.1.8-x86_64-pc-windows-msvc`),
preset CMake `clang23-debug`. Voir `.vscode/settings.json` / `c_cpp_properties.json`.

Le Dockerfile/.dockerignore n'a de sens que pour un exécutable autonome qu'on veut lancer tel
quel dans un conteneur. Pour une librairie destinée à être buildée/utilisée en local
(ex. `DateLibrary`), ne pas en ajouter par défaut — demander si un cas d'usage conteneurisé est
réellement voulu avant d'en créer un.

## Build

```
cmake -S <Projet> -B <Projet>/build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build <Projet>/build
```

## CI (.github/workflows/build.yml)

- `build-linux` : build tous les projets ayant un `CMakeLists.txt`, **sauf `CrashDump`**
  (exclu explicitement — voir plus bas).
- `build-windows` : build uniquement `CrashDump` (dépend de `dbghelp.h`, spécifique Windows).

## Inventaire des projets

- **CrashDump** — voir section dédiée ci-dessous, cas particulier.
- **DateLibrary** — seul projet du repo structuré comme une vraie librairie (cible CMake
  `datelib` séparée de l'exécutable de démo `DateLibraryDemo`, API publique sous `include/`,
  **pas de Dockerfile** — usage local uniquement, voir remarque ci-dessus). Type `Date` immuable
  construit au-dessus de `std::chrono` (calendrier grégorien, arithmétique de jours/mois/années
  avec troncature explicite en fin de mois, parsing/format ISO 8601, exception dédiée
  `InvalidDateException`). Pensé comme terrain d'exercice pour la conception d'API (séparation
  interface/implémentation, types forts, immutabilité, `operator<=>` défaulté).
- **HelloWorld** — sanity check minimal du toolchain (C++23 `std::to_underlying`).
- **Hashcode** — génération de collisions de hash sur une classe custom, avec timeout via
  `std::async`.
- **LockFreeStack** — pile lock-free avec `std::atomic` et `compare_exchange_weak`.
- **MonteCarlo** — estimation de π par méthode de Monte-Carlo, multithreadée.
- **NthRootProject** — calcul de racine n-ième par méthode de Newton, lecture clavier.
- **SharedPtr** — réimplémentation pédagogique de `shared_ptr`/`weak_ptr` avec control block
  atomique (compteurs thread-safe).
- **ThreadPool** — pool de threads maison avec `std::packaged_task` / `std::future`.
- **VTable** — exploration de l'héritage multiple et du layout des vtables (offsets de pointeurs
  entre bases lors d'un `static_cast` implicite).

## CrashDump — règle spéciale, ne pas "corriger"

Ce projet **doit crasher**. Le déréférencement de pointeur nul dans `main.cpp` est intentionnel :
le but est de générer un crash dump Windows (`crash.dmp` via `MiniDumpWriteDump`/`dbghelp.h`) et
de s'entraîner à l'analyser avec un debugger (PDB, WinDbg, etc.).

- Ne jamais "fixer" le crash, ajouter un null-check, ou neutraliser le comportement pour que le
  programme se termine proprement — ce serait contraire au but de l'exercice.
- C'est pour cette raison qu'il est explicitement exclu du job `build-linux` de la CI (il est
  spécifique à Windows via `dbghelp.h`) et n'a pas vocation à "réussir" comme les autres projets.
- Les demandes de travail sur ce projet concernent typiquement : le mécanisme du crash handler
  (`SetUnhandledExceptionFilter`), le contenu/la génération du `.dmp`, ou l'analyse post-mortem
  du dump (PDB, symboles, pile d'appel) — pas la suppression du bug.
- Si une modification est demandée qui rendrait le crash non reproductible, le signaler avant de
  l'appliquer plutôt que de la faire silencieusement.

## Style du code

Les commentaires dans les `CMakeLists.txt` sont volontairement didactiques (expliquent le rôle de
chaque commande CMake) — c'est un choix pédagogique délibéré, à conserver si on modifie ces
fichiers. Le code source, lui, est peu commenté ; il n'est pas nécessaire d'ajouter des
docstrings/commentaires systématiques dans le style du projet.
