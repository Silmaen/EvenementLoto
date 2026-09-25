# Événement Loto

Logiciel de gestion d’événement de type loto associatif.

## Utilisation

[Utilisation](document/Utilisation.md)

## Construction

Ce projet utilise CMake (version 3.24 ou supérieure) pour se configurer, à travers les
presets définis dans `CMakePresets.json`.

### Variables de construction

* `EVL_TESTING` construit les tests unitaires (activé par défaut).
* `EVL_PACKAGING` active la génération du paquet par CPack (activé par défaut).
* `EVL_ENABLE_COVERAGE` active ou non le calcul de la couverture de code.
  (nécessite `gcovr` voir dépendances)
* `EVL_ENABLE_CLANG_TIDY` active ou non l’utilisation de clang-tidy durant la compilation.
* `EVL_ENABLE_ADDRESS_SANITIZER` active ou non l’utilisation de l’`address sanitizer` durant la compilation.
* `EVL_ENABLE_THREAD_SANITIZER` active ou non l’utilisation du `thread sanitizer` durant la compilation.
* `EVL_ENABLE_LEAK_SANITIZER` active ou non l’utilisation du `leak sanitizer` durant la compilation.
* `EVL_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER` active ou non l’utilisation de l’`undefined behavior` sanitizer durant la
  compilation.

### Dépendance

Tout est piloté depuis CMake : `cmake --preset <preset>` puis `cmake --build` suffisent,
il n’y a aucune commande à lancer avant.

* **Python 3.12 ou supérieur** et [Poetry](https://python-poetry.org).
  Poetry est le seul prérequis Python : au moment du `cmake --preset`, CMake exécute
  `poetry sync` qui crée l’environnement virtuel et y installe l’outillage verrouillé
  par `poetry.lock` — **Conan**, `gcovr` (rapport de couverture) et `black`.
* **Conan 2** gère les dépendances C++, mais n’est jamais invoqué à la main :
  `cmake/conan_provider.cmake` en fait un *dependency provider* de CMake, déclenché au
  premier `find_package()`. Les profils sont dans `conan/config/profiles/` et c’est
  CMake qui lui transmet le compilateur et sa version.
    * glfw, imgui, jsoncpp, yaml-cpp, spdlog, magic_enum, stb, nanosvg,
      vulkan-headers, vulkan-loader, googletest — depuis ConanCenter
* **Doxygen** 1.9.1 ou supérieur, avec le module `dot` (paquet graphviz), pour la
  documentation de code. Non indispensable à la génération du logiciel, mais pouvoir
  produire une documentation reste fondamental.
* Sous Linux, les **bibliothèques de développement X11/XCB et Wayland** du système sont
  nécessaires à la compilation ; les images Docker de construction les fournissent.

### Construction dans un conteneur

La CI compile dans `registry.argawaen.net/builder/builder-ubuntu2404`, qui contient les
deux chaînes de compilation (gcc et clang) ainsi que tout l’outillage. Les recettes de
ces images vivent dans un dépôt séparé.

### Compilateur

Le programme a été correctement compilé avec :

* windows
    * gcc 14 (mingw)
    * clang 21 (mingw)

* linux ubuntu 24.04
    * gcc 14
    * clang 18 et 22

Sous Linux, X11 et Wayland sont tous deux supportés, mais **X11 est demandé par
défaut**, XWayland compris : le protocole Wayland interdit à une application de placer
ses propres fenêtres, donc d’envoyer l’écran d’affichage sur le vidéoprojecteur. Le
réglage `gui/display_server` change ce choix ; voir
[Utilisation](document/Utilisation.md).

## Versions

Ce qui est livré est dans [CHANGELOG.md](CHANGELOG.md), ce qui est prévu dans
[ROADMAP.md](ROADMAP.md).
