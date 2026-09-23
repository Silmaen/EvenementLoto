# TODO — Migration technique et stabilité

**Partie I** — migration DepManager → Conan, CI TeamCity en sources, retrait de Copilot.
**Partie II** — durcissement de la stabilité (4 h sans crash, reprise immédiate sur incident).
Les deux parties sont **indépendantes** et peuvent avancer en parallèle.

> Document de travail vivant. Chaque case cochée = fait et validé.
> Les phases sont ordonnées : **ne pas sauter une phase**, chacune isole une cause
> de panne. Les phases 0 à 2 sont indépendantes de Conan.

**État global** : 🟩 migration : phases 0 à 5, 8 et 11 (Linux), 7 quasi complète — stabilité : S0 à S6
**Dernière mise à jour** : 2026-09-21

---

## Conventions de travail

- **Commits** : messages très courts, à l'impératif. **Jamais de `Co-Authored-By`**
  ni d'autre trailer d'attribution.
- **Code** : simple et moderne, commentaires au strict nécessaire, **zéro code mort**
  — nettoyer après chaque aller-retour d'expérimentation.
- **Documentation** : Markdown, en deux jeux distincts. Doc **développeur** dans le
  dépôt ; doc **utilisateur** dans `document/Utilisation.md` (+ `document/images/`),
  intégrée aux pages d'aide de l'application. Ne pas mélanger les deux.

---

## Sommaire

**Partie I — Migration**

1. [Décisions arrêtées](#décisions-arrêtées)
2. [Faits vérifiés](#faits-vérifiés-ne-pas-re-chercher)
3. [Phase 0 — Hygiène et secrets](#phase-0--hygiène-et-secrets)
4. [Phase 1 — Élagage Vulkan + réconciliation image Clang](#phase-1--élagage-vulkan--réconciliation-image-clang)
5. [Phase 2 — Poetry seul maître du Python](#phase-2--poetry-seul-maître-du-python)
6. [Phase 3 — Recette locale nfd](#phase-3--recette-locale-nfd)
7. [Phase 4 — Bascule Conan (Linux)](#phase-4--bascule-conan-linux)
8. [Phase 5 — Presets qualité](#phase-5--presets-qualité)
9. [Phase 6 — MinGW](#phase-6--mingw)
10. [Phase 7 — Suppression de DepManager](#phase-7--suppression-de-depmanager)
11. [Phase 8 — CI TeamCity en Kotlin DSL](#phase-8--ci-teamcity-en-kotlin-dsl)
12. [Phase 9 — Tout en statique](#phase-9--tout-en-statique)
13. [Phase 10 — Retrait de nfd](#phase-10--retrait-de-nfd-au-profit-dun-sélecteur-intégré)
14. [Risques ouverts](#risques-ouverts)
15. [Annexes](#annexes)

**Partie II — Stabilité** (menable en parallèle, aucun recouvrement avec le build)

16. [Phase S0 — Ne plus perdre le log du crash](#phase-s0--ne-plus-perdre-le-log-du-crash)
17. [Phase S1 — Écritures atomiques et rotation](#phase-s1--écritures-atomiques-et-rotation)
18. [Phase S2 — Lecture défensive du format binaire](#phase-s2--lecture-défensive-du-format-binaire)
19. [Phase S3 — Reprise après incident au démarrage](#phase-s3--reprise-après-incident-au-démarrage)
20. [Phase S4 — Filet global contre les exceptions](#phase-s4--filet-global-contre-les-exceptions)
21. [Phase S5 — Autosave déclenché par les événements métier](#phase-s5--autosave-déclenché-par-les-événements-métier)
22. [Phase S6 — Robustesse GPU et session longue](#phase-s6--robustesse-gpu-et-session-longue)
23. [Phase S7 — Format de fichier portable](#phase-s7--format-de-fichier-portable)
24. [Phase S8 — Qualité et cohérence](#phase-s8--qualité-et-cohérence)
25. [Traçabilité des constats](#traçabilité-des-constats)

---

## Décisions arrêtées

| # | Sujet | Décision | Date |
|---|---|---|---|
| D1 | Pilotage | **CMake pilote Conan**, jamais l'inverse. `cmake --preset` + `cmake --build` restent la seule interface. Aucun `conan install` manuel. | 2026-09-21 |
| D2 | Intégration | `cmake-conan` **release 0.19.0**, `conan_provider.cmake` **copié** dans `cmake/`. Cette release utilise le générateur **stable `CMakeDeps`** (et non `CMakeConfigDeps` de `develop2`) : pas de fonctionnalité expérimentale, Conan ≥ 2.0.5 suffit. | 2026-09-21 |
| D3 | Profils Conan | Écrits à la main et versionnés. Pas d'autodétection (elle ne couvre que Win+MSVC / Linux+gcc / Apple+clang). | 2026-09-21 |
| D4 | Doxygen | **Reste une dépendance externe** de l'image Docker. Pas de `tool_requires`. | 2026-09-21 |
| D5 | Python | **Poetry gère les venv ET les dépendances**, conan inclus. Suppression du bricolage `.env` / `VENV_PATH`. | 2026-09-21 |
| D6 | nfd | **Recette Conan locale** pour démarrer (`local-recipes-index`), puis retrait complet de la dépendance en phase 10. | 2026-09-21 |
| D7 | Liaison | Phase 4 **reproduit à l'identique** le découpage shared/static de `depmanager.yml`. La bascule tout-statique est une phase séparée (9). *Révisé le 2026-09-22 : phase 9 faite, tout est statique sauf le loader Vulkan — c'était la condition pour que l'édition de liens MinGW aboutisse.* | 2026-09-21 |
| D8 | CI | **Kotlin DSL**, versionné au niveau du projet *Evenement Loto*. Le projet racine reste géré par l'UI (c'est là que remonte la clé SSH). | 2026-09-21 |
| D9 | Remote Conan | **Aucun**. `--build=missing` + cache `~/.conan2` persistant via le montage `/home/user`. | 2026-09-21 |
| D10 | Copilot | Retiré du projet. | 2026-09-21 |
| D11 | Conteneurs Linux | **Ubuntu 24.04** (déjà le cas). Les images 26.04 sont hors périmètre. | 2026-09-21 |

---

## Faits vérifiés (ne pas re-chercher)

### Correspondance des dépendances

| `depmanager.yml` | Paquet Conan Center | `find_package()` | Cible CMake | Action |
|---|---|---|---|---|
| `glfw 3.4.0` shared | `glfw/3.4` | `glfw3` | `glfw` | inchangé |
| `googletest 1.17.0` static | `gtest/1.17.0` | `GTest` | `GTest::gtest` | inchangé |
| `jsoncpp 1.9.6` static | `jsoncpp/1.9.6` | `jsoncpp` | `JsonCpp::JsonCpp` | **renommer** (`jsoncpp_static`) |
| `yaml-cpp 0.8.0` static | `yaml-cpp/0.8.0` | `yaml-cpp` | `yaml-cpp::yaml-cpp` | inchangé |
| `spdlog 1.17.0` shared | `spdlog/1.17.0` | `spdlog` | `spdlog::spdlog` | inchangé |
| `magic_enum 0.9.7` | `magic_enum/0.9.7` | `magic_enum` | `magic_enum::magic_enum` | inchangé |
| `stb_image 2.28` | `stb/cci.20240531` | `stb` | `stb::stb` | **renommer** |
| `nanosvg 1.0.0` static | `nanosvg/cci.20231025` | `nanosvg` | `nanosvg::nanosvg` | **renommer** |
| `imgui 1.92.5-docking` shared | `imgui/1.92.9b-docking` | `imgui` | `imgui::imgui` | **+ backends à compiler** |
| `nfd 1.2.1` static | *absent de CCI* | `nfd` | `nfd::nfd` | **recette locale** |
| `vulkan_sdk 1.4.328` shared | `vulkan-headers/1.4.350.0` + `vulkan-loader/1.4.350.0` | `VulkanHeaders`, `VulkanLoader` | `Vulkan::Headers`, `Vulkan::Loader` | inchangé |

### À retenir

- **Les noms Vulkan concordent** : la recette `vulkan-loader` déclare
  `cmake_file_name=VulkanLoader` / `cmake_target_name=Vulkan::Loader`, et
  `vulkan-headers` déclare `VulkanHeaders` / `Vulkan::Headers`. `cmake/Vulkan.cmake`
  n'a rien à changer sur cette partie.
- **`nativefiledialog-extended` n'est PAS sur Conan Center** (404 sur
  `recipes/nativefiledialog-extended/config.yml`). Le seul `nativefiledialog/116`
  présent est le fork *mlabbe*, avec une recette encore en Conan 1
  (`from conans import ...`) — API différente, inutilisable.
- **imgui CCI ne livre pas les backends compilés** : `imgui_impl_*.cpp/h` sont
  copiés en *sources* dans `res/bindings/` du paquet. À compiler nous-mêmes.
- **spirv-cross / shaderc / glslang / SPIRV-Tools / VulkanUtilityLibraries sont
  liés mais jamais utilisés** : `grep -rn "shaderc\|spirv" source/` ne renvoie rien,
  et le seul en-tête Vulkan inclus est `<vulkan/vulkan.h>`.
- **`EVL_DEFINE_VULKAN_LAYERS` et `EVL_BUILD_SHARED` ne sont définis nulle part**
  (`cmake/Vulkan.cmake:15`, `cmake/Vulkan.cmake:28`, `test/CMakeLists.txt:51`) → code mort.
- **`_lib` et `_ui` sont STATIQUES** (`add_library(… ${SRCS})` sans `SHARED`,
  `BUILD_SHARED_LIBS` non défini) → un seul exemplaire des globales imgui par exécutable.
- **Tous les flags du projet passent par la cible interface `EvenementLoto_Base`**
  (`-Werror -Weverything`, `--coverage`, sanitizers) — jamais globalement. Un
  sous-projet tiers ne les hérite donc pas.
- `gcovr.cfg` : `filter = .*source/core/.*|.*source/gui.*` → tout code tiers hors
  `source/` est déjà exclu de la couverture.

### cmake-conan / Conan

- Le provider s'installe via `CMAKE_PROJECT_TOP_LEVEL_INCLUDES`, **sans modifier
  les `CMakeLists.txt`** : les `find_package()` existants sont interceptés.
- `conan install` est déclenché **au premier `find_package()`**, avec
  `-of=${CMAKE_BINARY_DIR}/conan`. Sans `layout()` dans le conanfile, le dossier de
  générateurs est donc exactement `${CMAKE_BINARY_DIR}/conan`.
- **Aucune liste d'exclusion** : *tous* les `find_package()` passent par le provider.
- Le provider 0.19.0 exige le générateur **`CMakeDeps`** (stable). Conan verrouillé sur
  **2.32.0** par `poetry.lock`.
- `auto-cmake` écrirait `compiler.cppstd=gnu23` dans le profil généré (il lit
  `CMAKE_CXX_STANDARD`) : raison de plus pour n'utiliser que nos profils.
- Les include dirs des cibles `IMPORTED` sont traités comme `SYSTEM` par CMake par
  défaut → `-Weverything` ne devrait pas se déclencher sur les en-têtes tiers.

### nativefiledialog-extended (amont)

- Licence **Zlib**, 1068 ★, maintenu activement, **v1.4.0** disponible (projet épinglé sur 1.2.1).
- `NFD_PORTAL` **OFF par défaut** → backend **GTK3** (`libgtk-3-dev` déjà dans l'image).
  `libdbus-1-dev` n'est nécessaire **que** si `NFD_PORTAL=ON`.
- `NFD_WAYLAND` **ON par défaut** → `pkg_check_modules(WAYLAND REQUIRED wayland-client)`.
  → soit `libwayland-dev` dans l'image, soit `NFD_WAYLAND=OFF`.
- `NFD_BUILD_TESTS` / `NFD_INSTALL` valent `${nfd_ROOT_PROJECT}` → conçu pour
  l'inclusion, donc facile à empaqueter.

---

## Phase 0 — Hygiène et secrets

**Aucun impact build. À faire en premier.**

- [x] `.gitignore` : ajouter `TeamCity_*.zip` (le zip **n'est pas ignoré** aujourd'hui,
      un `git add .` le committerait)
- [x] `.gitignore` : ajouter `.teamcity/target/` (et `__pycache__/`) (sortie de build du DSL Kotlin)
- [x] Vérifier qu'aucun export TeamCity n'est déjà dans l'historique Git — **aucun**
      (`git log --all --diff-filter=A --name-only | grep -i zip`)
- [x] **Rotation des secrets** exportés en clair ou en `zxx` (brouillage réversible),
      faite le 2026-09-23 :
  - [x] clé privée SSH `github connexion` (était en clair dans le zip) — la clé
        publique du serveur est désormais une ed25519
  - [x] clé privée + client secret + webhook secret de la GitHub App « Owl »
  - [x] `github_access_token` (paramètre racine + celui du commit-status-publisher)
  - [x] `deploy_passwd`, `remote_passwd`
- [x] Clé SSH au projet racine TeamCity, **vérifié** : `_Root` la porte et les projets
      la voient par héritage, aucun doublon *(décidé : cohérent avec D8,
      le root reste géré par l'UI)*
- [x] Supprimer `.github/copilot-instructions.md` *(contenu déjà couvert intégralement
      par `CLAUDE.md` : tabulations, commentaires en anglais, préfixes `m_`/`i`/`o`/`io`,
      trailing return types, `log_error`/`log_warn`/`log_info`)*
- [x] Supprimer les `.idea/copilot.data.migration.*.xml` en local
      *(4 fichiers, déjà non suivis : `.idea/.gitignore` contient `copilot.*`)*
- [x] **Vérifié le 2026-09-23, rien à faire côté GitHub** : aucun fichier `copilot`
      suivi, aucun des deux rulesets ne demande de revue Copilot
      (`automatic_copilot_code_review_enabled` absent de la règle `pull_request`), et le
      code scanning n'est pas configuré, donc pas d'Autofix. Le reste est un réglage de
      **compte** (github.com/settings/copilot), pas de dépôt
- [x] Vérifier si `Python3_EXECUTABLE` est utilisé quelque part — **non**, seul usage :
      son propre `message(STATUS)` dans `cmake/Python.cmake:25` ⇒ suppression en phase 2
      → si non, `find_package(Python3)` (`cmake/Python.cmake:3`) sera supprimé en phase 2
- [x] ~~Vérifier la recette `doxygen` sur Conan Center~~ — sans objet (D4 : reste externe,
      et l'image `builder` fournit déjà doxygen 1.9.8 + graphviz) *(pour mémoire uniquement : D4
      tranche pour l'externe, cette case est là pour clore la question)*

**Validation** : aucune, phase sans effet sur le build.

---

## Phase 1 — Élagage Vulkan + réconciliation image Clang

**Encore sous DepManager.** Objectif : réduire la surface avant de toucher au
gestionnaire de paquets. Cette phase supprime à elle seule 11 dépendances.

- [x] `cmake/Vulkan.cmake` : réduire à `find_package(VulkanHeaders)` +
      `find_package(VulkanLoader)` + `target_link_libraries(… Vulkan::Loader)`
- [x] Supprimer `find_package` / liaisons : `VulkanUtilityLibraries`
      (`Vulkan::LayerSettings`, `Vulkan::UtilityHeaders`), `spirv_cross_*` (7 appels),
      `SPIRV-Tools`, `SPIRV-Tools-opt`, `glslang`, `shaderc`
- [x] Supprimer la branche morte `EVL_DEFINE_VULKAN_LAYERS`
- [x] Supprimer la branche morte `EVL_BUILD_SHARED`, et le bloc Qt mort de
      `test/CMakeLists.txt` (`EVL_QT_DIR` n'existe plus)
- [x] ~~`depmanager.yml`~~ : rien à retirer, `vulkan_sdk` est une entrée unique qui
      fournit headers + loader
- [x] `ci/PresetsParameters.json` : noms d'images réconciliés. Les images ont depuis été
      réorganisées en trois couches (`base` / `builder` / `devel`) sans le compilateur
      dans le nom : **les 7 presets Linux pointent maintenant sur `builder-ubuntu2404`**,
      qui contient gcc **et** clang.
- [x] Absorber les nouveaux diagnostics **clang 22** — **aucun** : clang 22.1.3 compile
      propre sous `-Werror -Weverything` (risque R3 levé)
- [x] ~~Relever `EVL_CLANG_MINIMAL`~~ : inutile, clang 18.1.3 compile toujours — le
      minimum déclaré reste honnête
- [x] **Supprimer le chemin Qt mort de `source/main.cpp`** *(+ le réglage `general/use_imgui`,
      et `main` retourne désormais `EXIT_FAILURE` sur `State::Error`)* : `USE_QT` n'est défini
      nulle part (ni CMake, ni presets) et `source/gui_qt/` **n'existe plus** — les
      blocs des lignes 9-27 et 63-82 incluent des en-têtes absents.
      ⚠️ Effet de bord réel : `main.cpp:53` lit
      `getValue<bool>("general/use_imgui", false)`, donc **un fichier de réglages
      contenant `use_imgui: false` fait sortir l'application en `EXIT_FAILURE`**
      au démarrage. Supprimer la branche et le réglage.
- [x] *(fait en phase 2)* Option `RSH_USE_PYTHON_VENV` supprimée avec `cmake/Python.cmake`
- [x] Mettre `CLAUDE.md` à jour (point d'entrée, version Clang du CI)
- [x] Doc développeur : nouveau flux de build décrit dans `README.md` et `CLAUDE.md`

> **Régression trouvée et corrigée** : `find_package(VulkanHeaders)` était appelé mais
> sa cible **jamais liée** — le chemin d'en-têtes arrivait par accident via
> `VulkanUtilityLibraries`. En retirant celui-ci, la compilation est tombée sur le
> Vulkan **système** de l'image (1.3), d'où `VK_API_VERSION_1_4 was not declared`.
> `Vulkan::Headers` est désormais lié explicitement. À reproduire en phase 4.

**Validation**
- [x] `linux-gcc-debug` : configure + build + `ctest` OK
- [x] `linux-clang-debug` : configure + build + `ctest` OK (clang 22 **et** clang 18)
- [x] L'application démarre et affiche une partie — première exécution réelle le
      2026-09-23

---

## Phase 2 — Poetry seul maître du Python

**Encore sous DepManager.**

- [x] `pyproject.toml` : ajouter `conan = "^2.32"` au groupe `build` (verrouillé sur **2.32.0**)
- [x] `cmake/Poetry.cmake` : `option(EVL_USE_POETRY … ON)`
      ⚠️ **actuellement OFF et aucun preset ne l'active → le `poetry sync` ne tourne jamais**
- [x] `cmake/Poetry.cmake` : récupérer le chemin du venv via `poetry env info --path`
      et préfixer `ENV{PATH}` avec son `bin/` (ou `Scripts/` sous Windows)
- [x] Forcer les venv **hors du projet** via `POETRY_VIRTUALENVS_IN_PROJECT=false`
      posé par `Poetry.cmake` (pas de config poetry globale modifiée)

> **Piège rencontré** : un `.venv/` traînait à la racine, créé quand le projet était
> monté sur `/source/personnel/EvenementLoto` (le chemin du vieux `.env`). Poetry
> réutilise un `.venv` existant, et ses scripts portaient un shebang mort
> (`#!/source/personnel/…/python`) ⇒ `depmanager: cannot execute`. Le `.venv` a été
> supprimé ; avec `POETRY_VIRTUALENVS_IN_PROJECT=false` chaque environnement a
> désormais son venv sous son propre `$HOME`.
      ⚠️ le VCS root TeamCity a `agentCleanFilesPolicy=ALL_UNTRACKED` +
      `agentCleanPolicy=ALWAYS` → **un `.venv` in-project serait effacé à chaque build**.
      `$HOME` est déjà monté de façon persistante (`-v …/user:/home/user -e HOME=/home/user`).
- [x] Supprimer `cmake/Python.cmake` et son `include(Python)` dans `BaseConfig.cmake`
- [x] Supprimer `.env` + `cmake/Environment.cmake` + `include(Environment)`
      *(vérifié : `VENV_PATH` était le seul contenu et le seul usage)*
- [x] `RSH_USE_PYTHON_VENV` supprimée avec `Python.cmake`

**Validation**
- [x] `poetry sync --no-root` s'exécute bien au configure **en conteneur**
      *(hôte non testé : builds natifs interdits)*
- [x] `conan --version` ≥ 2.25 résolu depuis le venv Poetry — **2.32.0**
- [x] `gcovr --version` ≥ 8.5 résolu depuis le venv — **8.6** (et `depmanager` 0.5.2)
- [x] Le venv survit à deux builds TeamCity consécutifs — confirmé à l'usage

---

## Phase 3 — Recette locale nfd

- [x] Créer `conan/local-recipes/recipes/nfd/config.yml` (versions `1.4.0`, `1.2.1`)
- [x] Créer `conan/local-recipes/recipes/nfd/all/conanfile.py`
- [x] Créer `conan/local-recipes/recipes/nfd/all/conandata.yml` (URL + sha256)
- [x] Déclarer le remote `local-recipes-index` — fait par `cmake/Conan.cmake`, avec le
      chemin absolu du dépôt, donc rien de figé dans un fichier

> **Deux pièges**
> 1. Le remote exige la disposition `<racine>/recipes/<nom>/`, pas `<racine>/<nom>/`.
> 2. La révision d'un `local-recipes-index` **ne suit pas le contenu** : une recette
>    modifiée continue d'être servie depuis le cache, et `--update` n'y change rien.
>    `cmake/Conan.cmake` hache donc `conan/local-recipes/` et fait
>    `conan remove "nfd/*"` dès que le contenu bouge. Les fichiers sont aussi déclarés
>    en `CMAKE_CONFIGURE_DEPENDS`.

> **Version retenue : 1.2.1**, pas 1.4.0. La 1.4.0 ajoute les champs `title`,
> `acceptLabel` et `cancelLabel` aux structures d'arguments, ce qui casse les
> initialisations désignées de `FileDialog.cpp` sous `-Werror=missing-field-initializers`.
> La recette sait construire les deux ; passer à 1.4.0 est une amélioration à part.

**Validation**
- [x] `conan create` OK en linux-gcc (gcc 14) et linux-clang (clang 22), backend GTK3
- [x] `conan create` OK sous les profils MinGW : les deux configurations Windows
      construisent la recette et passent leurs 109 tests
- [x] Le dialogue d'ouverture fonctionne (un `.lev` chargé depuis l'interface le
      2026-09-23). Enregistrer et sélectionner un dossier restent à confirmer
      *(non vérifiable sans écran : à faire à la main)*
- [x] Les suppressions de `lsan_suppressions.txt` **ne servent rien aujourd'hui** :
      aucun test GUI n'atteint `FileDialog`, donc dbus n'est jamais initialisé et la
      suite passe sans elles (vérifié avec `LSAN_OPTIONS=""`). Elles ne redeviendraient
      utiles que si un test exerçait le dialogue natif. Leur retrait est déjà prévu avec
      celui de nfd (phase 10), inutile de le faire deux fois

---

## Phase 4 — Bascule Conan (Linux)

**Faite.** Découpage shared/static identique à `depmanager.yml` (D7) : `imgui`, `glfw`,
`spdlog` en partagé, le reste en statique — l'archive CPack contient exactement les
mêmes `.so` qu'avant (`libimgui.so`, `libglfw.so.3.4`, `libspdlog.so.1.17.0`,
`libvulkan.so.1.4.350`).

- [x] `cmake/conan_provider.cmake` — copie de la **release 0.19.0**
      (sha256 `d574ac4f1ad0784e743304fde4af6e6692191783afae4898974903b622928546`)
- [x] `conanfile.py` : générateur `CMakeDeps`, 11 dépendances, pas de `layout()`
      (donc dossier de générateurs = `${CMAKE_BINARY_DIR}/conan`)
- [x] `conanfile.py` → `generate()` : copie des backends imgui **et** de
      `imgui_stdlib.*` (ConanCenter ne livre que les sources)
- [x] `conan/config/global.conf` + 4 profils (`linux-gcc`, `linux-clang`,
      `windows-mingw-gcc`, `windows-mingw-clang`)
- [x] `cmake/Conan.cmake` : choix du profil, `conan config install`, remote local,
      purge des recettes locales modifiées
- [x] `CMakeLists.txt` : `CMAKE_PROJECT_TOP_LEVEL_INCLUDES` **avant** `project()`,
      et `LANGUAGES CXX C` (Conan a besoin d'un compilateur C : glfw est en C)
- [x] Presets : `CMAKE_C_COMPILER` explicite, pour ne pas mélanger gcc et clang
- [x] `cmake/BaseConfig.cmake` : `include(Conan)` remplace `include(Depmanager)`
- [x] `cmake/DocumentationConfig.cmake` : `BYPASS_PROVIDER` sur `find_package(Doxygen)`
- [x] `source/core/CMakeLists.txt` : `jsoncpp_static` → `JsonCpp::JsonCpp`
- [x] `source/gui/CMakeLists.txt` : `stb_image`→`stb`, `NanoSVG`→`nanosvg`, `ImGui`→`imgui`
- [x] Cible `EvenementLoto_imgui_bindings` (backends + `imgui_stdlib`), includes en
      `SYSTEM` et `CXX_CLANG_TIDY` vidé
- [x] `copy_shared_libraries()` **réécrite** — voir la phase 11 : sous Conan elle ne
      copiait en réalité rien (les cibles `INTERFACE IMPORTED` n'ont pas de `LOCATION`)

### Profils : ce que CMake pousse à Conan

Les profils sont des gabarits Jinja qui lisent des variables d'environnement posées par
`cmake/Conan.cmake` : `EVL_COMPILER_VERSION`, `EVL_C_COMPILER`, `EVL_CXX_COMPILER` et
`EVL_SYSTEM_PKG_CONFIG_PATH`. Un seul profil par famille de compilateur suffit donc, et
la version reste toujours celle que CMake utilise réellement.

### Obstacles rencontrés

1. **`compiler.cppstd` est obligatoire.** `spdlog` (avec `use_std_fmt`) et `gtest`
   refusent une configuration sans `cppstd`. Le plan initial (l'omettre pour récupérer
   les binaires précompilés de ConanCenter) ne tient pas → `compiler.cppstd=gnu23`,
   aligné sur le projet. **Conséquence assumée : 7 paquets se construisent depuis les
   sources** (glfw, imgui, jsoncpp, yaml-cpp, spdlog, gtest, nfd) ; les paquets
   d'en-têtes se téléchargent. Coût unique grâce au cache `~/.conan2` persistant.
2. **`xorg/system` réclame 31 paquets X11 dev** absents de l'image. Résolu en
   déclarant `[platform_requires] xorg/system` dans les profils Linux : X11 vient de
   la plateforme, ce qui est la réalité d'une image de build de bureau.
3. **Conan isole son `pkgconf`**, donc les `.pc` système devenaient invisibles et
   `vulkan-loader` ne trouvait plus `x11`. `cmake/Conan.cmake` interroge le
   `pkg-config` système (`--variable pc_path`) et le transmet via `[buildenv]`.
4. **`vulkan-loader` WSI** : d'abord limité à xlib faute de paquets système, puis
   **complété (xlib + xcb + wayland)** une fois l'image enrichie — voir la phase 11.
5. **`CMAKE_MAP_IMPORTED_CONFIG_DEBUG Release`** dans `Vulkan.cmake` (hérité du SDK
   DepManager qui n'avait que du Release) empêchait CMake de résoudre
   `IMPORTED_LOCATION` du loader. Supprimé : Conan construit un vrai Debug.
6. **imgui 1.92.5 → 1.92.9b** (1.92.5 n'est pas sur ConanCenter) : deux adaptations,
   `.ExtraDynamicStates = {}` dans `PipelineInfoMain` et
   `IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE` →
   `IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE`.
7. **nfd statique + GTK3** : les symboles `gdk_*` manquaient au lien final, la recette
   doit exposer `gdk-3` en plus de `gtk-3` dans `system_libs`.

**Validation**
- [x] `linux-gcc-debug` : configure + build + `ctest` (2/2)
- [x] `linux-clang-debug` : configure + build + `ctest` (2/2), profil `linux-clang`
- [x] `linux-clang-release` : build + `cpack` → `EvenementLoto-0.4.1-Linux-x64.tar.gz`
      avec les mêmes `.so` qu'avant la migration
- [x] Les en-têtes tiers ne déclenchent pas `-Weverything` (cibles `IMPORTED` ⇒ `SYSTEM`)
- [x] Un `cmake --preset` sur un cache Conan vide fonctionne (bootstrap complet)
- [x] L'application démarre et charge une partie — `super_loto.lev` chargé puis partie
      démarrée, le 2026-09-23

## Phase 5 — Presets qualité

- [x] `linux-sanitizer-address` : build + `ctest` (2/2)
- [x] `linux-sanitizer-leak` : build + `ctest` (2/2), `lsan_suppressions.txt` suffisant
- [x] `linux-sanitizer-thread` : build + `ctest` (2/2)
      ⚠️ TSan exige `--cap-add=SYS_PTRACE --security-opt seccomp=unconfined` sur le
      conteneur (l'ASLR le fait échouer sinon). La CI les passe déjà via
      `ci/actions/define_docker_image.py`.
- [x] `linux-sanitizer-undefined-behavior` : build + `ctest` (2/2)
- [x] `linux-clang-tidy` : **vert**
- [x] clang-tidy n'analyse plus les sources tierces

### clang-tidy 22 : 32 constats, tous traités

Le passage de clang-tidy 18 à 22 a fait remonter 32 diagnostics. Deux confirment
l'analyse de stabilité de la Partie II, de façon indépendante :

- `bugprone-exception-escape` sur `main` → **S4** : `main` est désormais protégé par un
  `try/catch`, et le gestionnaire de dernier recours est `noexcept`.
- `bugprone-throwing-static-initialization` (11×) → les tables de conversion statiques
  `g_typeConvert` / `g_statusConvert` et les globales `const std::string` pouvaient
  lancer **avant `main`**, sans rattrapage possible. Remplacées par des tables
  `constexpr` (`source/core/EnumLabel.h`) et des `constexpr std::string_view`.
  Effet de bord bienvenu : `Event::getStatusStr()` n'utilise plus un `.at()` non gardé,
  ce qui **retire un des chemins de crash identifiés en S2**.

Le reste : `readability-use-std-min-max` (2), `readability-inconsistent-ifelse-braces`
(2), `modernize-use-trailing-return-type` (5), `modernize-use-designated-initializers`,
`modernize-use-integer-sign-comparison`, `readability-redundant-member-init` (3),
`google-explicit-constructor`, `misc-override-with-different-visibility` (3),
`bugprone-random-generator-seed` (NOLINT : le tirage prévisible est volontaire en debug).

Deux décisions de configuration, dans `.clang-tidy` :

- `HeaderFilterRegex` restreint à `.*/source/.*` : il englobait tous les en-têtes hors
  `test/`, donc ceux des paquets Conan.
- `cppcoreguidelines-use-enum-class` désactivé : `event::Category` est un jeu de
  drapeaux de bits et `MouseCode` reprend les codes glfw, tous deux non scopés à dessein.

Et un changement de structure : les implémentations des bibliothèques à en-tête unique
(`stb_image`, `nanosvg`) vivent maintenant dans `source/third_party/implementations.cpp`,
au sein de la cible `EvenementLoto_third_party` qui porte aussi les backends ImGui.
Cette cible est exclue de clang-tidy et des avertissements du projet — deux diagnostics
de l'analyseur statique tombaient dans `nanosvg.h` alors qu'ils ne nous concernent pas.

## Phase 6 — MinGW

**Faite.** Agent Windows natif (décision : on garde le natif, MinGW et non clang-cl).
Les deux configurations passent : `WindowsX64_Gcc` et `WindowsX64_Clang` rapportent
`Tests passed: 109`, comme les sept configurations Linux.

- [x] Résolution du graphe Conan validée pour les deux profils Windows, **depuis Linux**
      (`conan graph info`) : 17 nœuds en gcc, 16 en clang, aucune recette ne refuse
      MinGW, aucun conflit de version
- [x] `cmake/CMakePresetsMinGW.json` : rien à changer, `CONAN_HOST_PROFILE` est choisi
      par `cmake/Conan.cmake` d'après le compilateur détecté
- [x] **Interblocage corrigé dans `ci/utils/run.py`** (voir ci-dessous) — confirmé :
      `conan install` résout et construit désormais toutes les dépendances Windows,
      `Configuring done (122.9s)`
- [x] Étape *Tool Dependencies* : une ligne commentée avec `#` faisait échouer le
      script `cmd` sous Windows (`'#' n'est pas reconnu…`). Aucune syntaxe de
      commentaire n'est portable entre bash et cmd : la ligne doit être supprimée.
- [x] **Trois échecs Windows corrigés**, tous issus de chemins non exécutables depuis
      Linux :
  1. `ci/utils/run.py` : interblocage sur les tubes (voir ci-dessous) ;
  2. `$<TARGET_RUNTIME_DLLS>` appliqué à des bibliothèques statiques ;
  3. `Log.cpp` : `getLogPath()` renvoie un `std::filesystem::path`, qui se convertit
     en `std::wstring` sous Windows, alors que le `filename_t` de spdlog est un
     `std::string` (`wchar_filenames=False`). Corrigé par `.string()`.
- [x] ⚠️ **`IMGUI_API` en DLL sous MinGW (risque R2) : confirmé et résolu.** Les 82
      symboles `undefined reference to __imp__Z…ImGui_Impl…` disaient tout : `imgui` en
      partagé marque `IMGUI_API` en `dllimport` pour les consommateurs, alors que les
      backends que **nous** compilons n'exportent rien. **`imgui` passe en statique**,
      ce qui donne aussi un exemplaire unique de ses globales dans l'exécutable — la
      préoccupation d'origine. `libimgui.so` disparaît de l'archive livrée.
- [x] `windows-gcc-debug` : configure + build + `ctest` — 109 tests
- [x] `windows-clang-debug` : configure + build + `ctest` — 109 tests
- [x] `windows-clang-release` + `cpack` : l'archive livrée ne contient plus que
      `EvenementLoto.exe` (4,1 Mo), les trois runtimes MinGW et `vulkan-1.dll`.
      *`windows-gcc-release` n'est pas construit : comme sous Linux, une seule
      configuration déploie (`run_deploy` dans `ci/PresetsParameters.json`).*
- [x] Copie des DLL MinGW vérifiée dans l'archive (`libgcc_s_seh-1.dll`,
      `libstdc++-6.dll`, `libwinpthread-1.dll`)
- [x] **Dernier échec : `evl_gui_test_UTests` s'arrêtait sur `0xc0000135`**
      (`STATUS_DLL_NOT_FOUND`) alors que les 93 tests de `evl_lib_test` passaient.
      `glfw3.dll` manquait à côté des exécutables : la cible importée que `CMakeDeps`
      déclare pour glfw n'expose pas d'`IMPORTED_LOCATION` sous MinGW, donc
      `$<TARGET_RUNTIME_DLLS>` ne la voyait pas. Réglé par la phase 9 (tout en
      statique), qui supprime la question au lieu de rafistoler la plomberie.
- [x] `$<TARGET_RUNTIME_DLLS>` : **corrigé**. Il n'accepte que les cibles exécutable,
      partagée ou module, alors que `copy_shared_libraries()` était appelée sur les
      bibliothèques **statiques** `_lib` et `_ui` — d'où quatre `CMake Error` sous
      Windows. Les DLL vont désormais à côté de l'exécutable et de chaque exécutable
      de test (sans quoi ils ne démarreraient pas), et la fonction sort d'elle-même
      si la cible n'est pas d'un type qui accepte des DLL à côté.

### L'interblocage : `conan install` figé sous Windows

Les deux builds Windows sont restés bloqués à l'étape *Build*, **1 h 35 sans une seule
ligne de sortie**, jusqu'à annulation manuelle. La cause était dans notre code, pas
dans Conan.

`ci/utils/run.py` avait deux chemins : sous Linux une boucle `select()` qui drainait les
deux flux, sous Windows une lecture **séquentielle** — `for line in process.stdout` puis
`for line in process.stderr`. Or `conan install` écrit toute sa progression sur
**stderr** (avec `--format=json`, stdout ne porte que le JSON, 629 Ko). Le tampon du
tube stderr se remplit, CMake bloque en écriture, `conan install` se fige
définitivement.

Ça n'était jamais apparu avant parce que DepManager produisait bien moins de sortie que
la construction à froid de toutes les dépendances par Conan.

Corrigé en drainant les deux flux **en parallèle, un thread chacun**, sur les deux
plateformes. Cela supprime la fourche Linux/Windows, la dépendance à `fcntl`/`select`, et
au passage une course sur l'état global du détecteur de niveau de log — désormais une
instance par flux.

> **Fausse piste, pour mémoire** : j'avais d'abord cru à un simple problème de
> visibilité et ajouté `ECHO_OUTPUT_VARIABLE` au `conan_provider.cmake` vendoré. Le test
> a montré que, avec `--format=json`, stdout ne contient **que** le JSON : ce correctif
> aurait déversé 629 Ko de JSON dans chaque log. Annulé.

## Phase 7 — Suppression de DepManager

- [x] Supprimer `cmake/Depmanager.cmake`
- [x] Supprimer `depmanager.yml`
- [x] `pyproject.toml` : retirer `depmanager = "^0.5.1"`
- [x] `poetry.lock` : régénérer
- [x] TeamCity : `poetry run dmgr remote add …` retiré du runner « Tool Dependencies »
      *(fait dans le DSL Kotlin, phase 8)*
- [x] **Sans objet.** `remote_url`, `remote_login`, `remote_passwd` et
      `github_access_token` vivent sur le projet racine et servent à **d'autres
      projets** ; ils n'ont donc pas à disparaître. Ce que la phase 7 exigeait est
      vérifié : plus aucune référence dans le dépôt, et aucune dans les étapes de build
      du projet. Ce qu'on voit dans la liste des paramètres du projet est la vue
      **héritée** du racine, comme pour la clé SSH
- [x] **Dépôt `CI/DockerImages`** : `pip install … depmanager gcovr` retiré de
      `_common/builder.sh`
- [x] Images reconstruites et publiées *(fait côté dépôt DockerImages)*
- [x] Mettre à jour `CLAUDE.md` (sections « Dependency management », « External
      Dependencies », « Python Dependencies », « Build System », `conan/`, cibles)
- [x] Mettre à jour `README.md` : dépendances, pilotage par CMake, conteneur de build

---

## Phase 8 — CI TeamCity en Kotlin DSL

**Faite.** `.teamcity/settings.kts` (format portable) + `.teamcity/pom.xml` repris
verbatim de l'export du serveur, et `.teamcity/README.md` pour la procédure.

- [x] Squelette obtenu depuis le serveur (*Versioned Settings → Download settings in
      Kotlin format*), qui fixe `version = "2026.2"` et les coordonnées du plugin Maven
- [x] Aucun secret dans le dépôt : le projet racine reste géré par l'interface et porte
      les paramètres partagés, la connexion GitHub App et la clé SSH. Le VCS root ne
      référence la clé que **par son nom**.
- [x] Les 9 configurations sont produites par une fabrique `presetBuild()` : elles ne
      diffèrent que par `cmake_preset`
- [x] **`id` posés explicitement** (`RelativeId("LotoBranch_Build_LinuxX64_Gcc")`, …) et
      identiques à ceux du serveur — les changer ferait perdre l'historique de build
- [x] `ci/PresetsParameters.json` reste la source de vérité unique ; le DSL ne duplique
      ni `run_tests`, ni `run_coverage`, ni `release_preset`, ni `docker_image`
- [x] Les 11 étapes du template sont reproduites, conditions incluses
- [x] Extensions conservées : `investigationsAutoAssigner`, `xmlReport` (gtest,
      `output/build/**/test/*_Report.xml`), `perfmon`, `github-bridge`
- [x] ~~Pas de trigger VCS~~ — **faux, corrigé le 2026-09-22** : le pont ne met en file
      que sur événements de pull request, son contrôleur webhook ignore `push`. `main` a
      donc un `vcsTrigger` limité à `+:main` (voir 12.0)
- [x] Synchronisation activée, et **vérifiée** : le statut du projet dit « Changes from
      VCS are applied to project settings », format kotlin
- [x] ~~Comparer un build avant/après sur la même révision~~ — **sans objet** : le mode
      a été passé à *always use current settings*, donc un build de branche ne charge
      jamais les settings du VCS. Ce qui le remplace est le diff du XML généré contre
      celui de `main`, fait à chaque changement du DSL

### Validation : le DSL compile et régénère le XML

Compilé dans un conteneur `maven:3.9-eclipse-temurin-21`
(`mvn teamcity-configs:generate`), puis le XML produit a été comparé fichier par fichier
à l'export du serveur. **8 des 16 fichiers sont strictement identiques**, et les 8 autres
ne diffèrent que par les nettoyages voulus :

- la ligne `dmgr remote add` de l'étape « Tool Dependencies » *(phase 7)* ;
- le paramètre `WatchBranchFilter`, plus référencé depuis la disparition du trigger VCS ;
- `disabled-settings` renvoyant à `BUILD_EXT_7` et `TRIGGER_2`, deux identifiants qui
  n'existent plus ;
- les `perfmon` redéclarés par configuration alors que le template les fournit déjà.

### Deux pièges rencontrés

1. **Un `object` de script Kotlin ne peut pas capturer l'instance du script**, donc pas
   appeler une fonction de niveau script (« captures the script class instance »). Tout
   est donc déclaré en `val` avec un `id` explicite, ce qui est de toute façon plus sûr
   pour l'historique.
2. **L'export Kotlin du serveur perd des réglages.** Il a laissé tomber
   `ignoreKnownHosts`, `agentCleanFilesPolicy` et `submoduleCheckout` sur le VCS root —
   or sans `ignoreKnownHosts` l'agent refuserait la clé d'hôte de GitHub — ainsi que
   quatre paramètres du pont GitHub (`annotateDiff`, `publishChecks`, `triggerOnBranch`,
   `triggerOnPrReady`) et le `triggerOnPrDraft` propre aux deux configurations Clang.
   Tous rétablis explicitement, ce qui rend le XML régénéré fidèle.

## Phase 9 — Tout en statique

**Faite**, et pas par choix esthétique : les deux échecs Windows de la phase 6 étaient
deux symptômes du même problème.

1. `imgui` partagé marque `IMGUI_API` en `dllimport` pour ses consommateurs, alors que
   les backends que l'on compile soi-même n'exportent rien — 82 symboles `__imp_…`
   manquants à l'édition de liens MinGW.
2. `glfw` partagé n'a jamais déposé sa DLL à côté des exécutables de test : sa cible
   importée n'expose pas d'`IMPORTED_LOCATION` exploitable par `$<TARGET_RUNTIME_DLLS>`
   sous MinGW, et `evl_gui_test_UTests` mourait en `0xc0000135`
   (`STATUS_DLL_NOT_FOUND`).

Le projet produit **un seul exécutable** à partir de bibliothèques statiques : rien ne
justifiait de livrer des DLL, et l'état global d'ImGui n'existe plus qu'en un exemplaire.

- [x] `conanfile.py` : `shared=False` partout sauf `vulkan-loader`
      (`package_type = "shared-library"` par nature)
- [x] Garde-fou : inutile. `BUILD_SHARED_LIBS` n'est jamais activé et `_lib`/`_ui` sont
      statiques par construction — c'est justement ce qui rend la bascule sûre.
      *(si `_lib`/`_ui` devenaient un jour partagées, repasser `imgui` en `shared=True`)*
- [x] `copy_shared_libraries()` : **conservée**. Elle reste nécessaire sous Windows pour
      `vulkan-1.dll` et les runtimes MinGW (`libgcc_s_seh-1`, `libstdc++-6`,
      `libwinpthread-1`).
- [x] Blocs `GET_RUNTIME_DEPENDENCIES` : **conservés**, pour `libvulkan.so.1` sous Linux.
- [x] Revalider l'install et CPack
- [x] Vérifier la taille du binaire : 3,9 Mo en release (statique, sans les données)

**Effet de bord traité** : `xorg/system` déclare l'intégralité de X11 dans ses
`system_libs`. glfw statique faisait donc enregistrer **68 `DT_NEEDED`** à l'exécutable,
dont une soixantaine de bibliothèques jamais appelées (`libXaw.so.7`, `libXv.so.1`,
`libXRes.so.1`, `libxkbfile.so.1`…) — absentes d'un bureau standard, l'application
aurait refusé de démarrer. `-Wl,--as-needed`
(`cmake/BaseConfig.cmake:127`) ramène le compte à **10** : `libvulkan`, `libgtk-3`,
`libgdk-3`, `libgobject`, `libglib` et la libc. Tout ce qui touche au serveur
d'affichage (X11, XCB, Wayland, xkbcommon) est chargé en `dlopen` par soname par glfw,
donc fourni par l'hôte — c'est exactement ce qu'il faut pour la pile compositeur.

L'archive CPack ne contient plus que l'exécutable, `libvulkan.so.1*`, `data/` et
`resources/`, et le binaire installé porte un `RPATH` réduit à `$ORIGIN`.

---

## Phase 10 — Retrait de nfd au profit d'un sélecteur intégré

**Objectif** : supprimer la dernière dépendance non disponible sur Conan Center, et
obtenir un sélecteur *parfaitement adapté* : dessiné en ImGui, thémable via la classe
`Theme`, sans dépendance système (GTK / dbus / wayland), **testable** dans
`test/gui_test/`, et surtout sans modale native qui passe derrière ou vole le focus
de la fenêtre plein écran (`DisplayView`).

### Le point dur : synchrone → asynchrone

`source/gui/utils/FileDialog.h` expose trois fonctions **bloquantes** qui retournent
un `std::filesystem::path`. Un sélecteur ImGui s'étale sur plusieurs frames : il est
**forcément asynchrone**. Les 12 sites d'appel, tous de la forme
`if (const auto path = FileDialog::openFile(f); !path.empty()) { … }`, doivent devenir
des continuations.

- [ ] Concevoir l'API asynchrone (callback, ou état + interrogation par frame)
- [ ] Implémenter le widget (arborescence, filtres, saisie du nom, confirmation
      d'écrasement, navigation clavier, tri, dossiers cachés, **lettres de lecteur
      sous Windows**)
- [ ] Reprendre le format de filtre existant `"Nom|ext1,ext2\nNom2|ext"`
      (`g_gameFilter`, `g_imageFilter`, `g_yamlFilter`)
- [ ] Conserver `m_lastPath` (mémorisation du dernier dossier)
- [ ] Convertir les 3 appels de `source/gui/actions/FileActions.cpp` (l.31, 59, 78)
- [ ] Convertir les 9 appels de `source/gui/views/ConfigPopups.cpp`
      (l.64, 333, 371, 400, 414, 600, 607, 807)
- [ ] Tests dans `test/gui_test/`
- [ ] Retirer `nfd` du `conanfile.py`, supprimer `conan/local-recipes/nfd/`
- [ ] Retirer les suppressions dbus de `lsan_suppressions.txt` si elles ne servent plus
- [ ] Retirer `libgtk-3-dev` / `libgtk-3-0t64` des images si plus aucun projet n'en dépend

> **Alternative écartée** : `portable-file-dialogs/0.1.0` *est* sur Conan Center
> (header-only, zéro dépendance de build, API synchrone conservée), mais sous Linux
> elle lance `zenity`/`kdialog` en sous-processus — dépendance à l'exécution
> inacceptable pour une appli distribuée en archive à des associations.
> `imfilebrowser.h` (AirGuanZ, MIT, en-tête unique) reste une base possible pour
> accélérer l'implémentation du widget.

---

## Phase 12 — CI : qualité, analyse sur diff, packager

Quatre chantiers demandés le 2026-09-22, après le merge de la migration. Références
internes : `stack_owl/Owl` (Code Style, matrice PR/draft) et `Sources/Test_CI`
(analyse sur diff, `ci/changed_tus.py` + `ci/clang_analysis.py`).

### 12.0 Déclenchement (fait, PR #50)

Le pont ne met en file que sur événements de pull request : son contrôleur webhook
ignore `push` et `branchTrigger.enabled` n'a aucun consommateur à l'exécution en 1.10.0.
`main` retrouve donc un `vcsTrigger` (`+:main`), les PR restent au pont, et une branche
poussée sans PR ne déclenche rien — c'est le comportement voulu.

- [x] `vcsTrigger` sur le template, `branchFilter = +:main`, comme Owl
- [x] `branch_specification` : **garder** `Feature/*` et `Experiment/*`. Ce n'est pas un
      déclencheur, c'est ce qui rend ces branches visibles à TeamCity, sans quoi les
      builds de PR (`prBuildRef = branch`) n'auraient aucune branche sur laquelle tourner

### 12.1 Un template léger pour les configurations hors build

`globalBuild` enchaîne 11 étapes (build, test, couverture, doc, deploy). Code Style,
Packager et Analysis n'en ont pas besoin. Il faut un second template avec les deux
seules étapes communes : `DefineDockerImage` sur l'agent, puis `PythonRequirements` +
`DefineVariables` dans le conteneur.

- [x] `val toolBuild = Template { ... }` : `Determine docker` + `Tool Dependencies`, plus
      le `vcsTrigger` factorisé dans `Triggers.mainBranchOnly()`
- [x] `ci_action.py` : laisser passer les options non reconnues jusqu'à l'action, pour
      que `Analysis` prenne des drapeaux (`--tool`, `--mode`, …) au lieu de variables
      d'environnement. `BaseAction.withOptions()` refuse par défaut toute option, afin
      qu'une option passée à une action qui n'en prend pas soit une erreur, pas un
      silence

### 12.2 Code Style

Une action `CodeStyle` qui **inspecte sans jamais réécrire**, et dont chaque constat
sort au format `chemin:ligne:colonne: error: <contrôle>: <message>` — c'est cette forme
que le pont transforme en annotation GitHub posée sur la bonne ligne du diff.

- [x] `clang-format --dry-run -Werror` sur `source/` et `test/`, un appel par fichier :
      groupés, clang-format s'arrête au premier fautif et ne rapporterait qu'un constat
- [x] ~~`cmake-format --check`~~ **abandonné.** Essayé, mesuré, retiré : avec
      `.cmake-format.json` tel qu'il est, l'outil éclate `target_link_libraries(X PUBLIC
      Y)` sur quatre lignes et casse les `file(GLOB_RECURSE)`. C'est moins lisible que la
      mise en forme à la main. Régler ce fichier est un chantier en soi ; d'ici là les
      sources CMake ne sont pas sous contrôle, et `cmakelang` n'est pas ajouté à Poetry
- [x] `black --check` sur `ci/` et `ci_action.py`
- [x] Configuration TeamCity `Code Style` dans le sous-projet *Quality*, sur `toolBuild`
- [x] Arbre normalisé : 17 fichiers reformatés, 263 insertions / 248 suppressions,
      commit séparé pour rester révocable

### 12.3 Distinction PR / PR draft

Owl ne construit pas la même chose selon l'état de la PR : les configurations lourdes
sont en `triggerOnPrDraft = false`, seul `Code Style` tourne aussi sur les brouillons.

- [x] `Code Style` : `triggerOnPrDraft = true` — c'est un contrôle de forme, il coûte
      quelques secondes et c'est précisément sur un brouillon qu'on veut le savoir tôt
- [x] Builds, tests et sanitizers : `triggerOnPrDraft = false`
- [x] Conséquence : retiré de `LinuxX64_Clang` et `WindowsX64_Clang` — c'étaient deux
      builds complets sur chaque poussée d'un brouillon. Le paramètre `triggerOnPrDraft`
      de `presetBuild()` n'avait plus d'appelant et a été supprimé avec

### 12.4 Analyse sur diff : clang-tidy et l'analyseur statique

Aujourd'hui clang-tidy tourne **pendant la compilation** (`CXX_CLANG_TIDY` via le preset
`linux-clang-tidy`) : tout ou rien, et impossible de le restreindre à un diff. Le modèle
de Test_CI sépare l'analyse de la compilation — elle configure son propre arbre pour
obtenir une base de compilation, puis appelle clang-tidy sur une liste de TU.

- [x] `ci/utils/changed_tus.py` : un en-tête modifié tire **toute** TU qui l'inclut,
      transitivement, sans quoi la porte laisserait passer une régression réelle
- [x] `ci/actions/analysis.py` : `--tool tidy|analyzer`, `--mode full|diff`,
      `--on-findings warn|fail`, `--merge-base`, `--base`
- [x] **Une plage de diff indéterminable échoue** au lieu de passer : sans base de
      fusion publiée et sans `origin/<base>` sur l'agent, une porte qui ne sait pas ce
      qu'elle couvre doit être rouge. Vérifié dans les deux modes
- [x] Preset `linux-analysis` produisant `compile_commands.json`, dans son propre
      répertoire de build. **Configure seulement** : la base de compilation est un
      produit de la génération, aucun objet n'est nécessaire pour analyser une TU
- [x] Quatre configurations : tidy et analyzer × (sur diff, `fail`, porte de PR) et
      (complète, `warn`, sur `main`). `Clang-Tidy` garde son id : c'est le même contrôle,
      mieux exécuté, et son historique vaut la peine d'être conservé
- [x] « Complète possible » : le mode est un paramètre, une exécution manuelle élargit
      une porte à tout le code sans qu'une configuration de plus existe pour ça
- [x] Mesuré : scan complet sur 38 TU, diff de 38 fichiers modifiés → 14 TU en 84 s.
      Aucun constat, ni en tidy ni en analyzer — vérifié non vide par un contrôle avec
      une vérification bruyante, qui remonte bien 16 diagnostics
- [x] La base du diff vient de `teamcity.github.bridge.pullRequest.mergeBase` publié par
      le pont, `pullRequest.targetBranch` en repli. Ces surcharges vont **sur la
      configuration**, jamais sur le template : une référence de paramètre que TeamCity
      ne peut pas résoudre devient une exigence d'agent implicite et le build ne démarre
      jamais
- [ ] **À vérifier côté serveur** : le réglage `mergeBase.enabled` du plugin doit être
      actif, sinon `pullRequest.mergeBase` arrive vide et l'analyse retombe sur
      `git merge-base HEAD origin/<base>`, qui exige cette branche dans le checkout.
      C'est un réglage serveur, hors de portée du compte `claude`
- [x] `annotateDiff = false` sur les analyses complètes : une trouvaille dans du code
      vieux de deux ans n'a rien à faire en commentaire sur les lignes d'une PR
- [x] Le `vcsTrigger` quitte le template léger : une analyse sur diff n'a rien à faire
      sur une poussée, son diff contre `main` y serait vide. Seuls `Code Style`, les
      analyses complètes et les `Package` le portent

### 12.5 Packager

Une configuration qui ne fait que produire l'archive prête à exécuter, et **ne tourne
pas sur les PR**.

- [x] `CPACK_PACKAGE_FILE_NAME` ⇒ `EvenementLoto-<version>`. `EVL_PLATFORM_STR` et
      `EVL_ARCH_STR` n'avaient plus d'autre usage et ont été supprimés de
      `cmake/BaseConfig.cmake`
- [x] Le générateur `TGZ` de CPack écrit `.tar.gz` ; l'action renomme en `.tgz`
- [x] Vérifié en conteneur : `EvenementLoto-0.4.1.tgz`, 5,7 Mo, contenant l'exécutable,
      `libvulkan.so.1*`, `data/` et `resources/` sous un seul dossier racine
- [x] Sous-projet *Package*, une configuration par plateforme, `artifactRules` sur la
      seule archive — pas de `BuildArtefact.zip` ni de `Coverage.zip`
- [x] `triggerOnPrReady = false` plutôt que pas de pont du tout : aucune PR ne le
      déclenche, mais la ligne de check apparaît quand même sur le commit empaqueté
- [x] `Deploy` retiré du template, avec tout son câblage : `ci/actions/deploy.py`,
      `run_deploy` dans `PresetsParameters.json`, `preset.py`, `define_variables.py`,
      la case à cocher du DSL et les deux lignes `*.zip` / `*.tar.gz` des artefacts de
      build. On empaquetait sinon deux fois le même programme

---

### 12.6 Dépendances entre configurations

Il n'y en avait **aucune**, nulle part. Modèle d'Owl : tout attend `Code Style`, en
`take-successful-builds-only` et sans démarrer du tout si la dépendance échoue.

- [x] `Dependencies.after(vararg gates)` : `reuseBuilds = SUCCESSFUL`,
      `onDependencyFailure = FAIL_TO_START`. Les options autres que `reuseBuilds` sont
      les valeurs par défaut du serveur et ne sont donc pas émises dans le XML — elles
      restent écrites dans le DSL, conformément au parti pris du fichier
- [x] Les 9 builds, les 4 analyses et les 2 `Package` attendent `Code Style`
- [x] **Divergence assumée avec Owl** : `Package` attend *aussi* la configuration Clang
      de sa plateforme, celle qui construit et teste le preset release empaqueté. Owl
      empaquette en parallèle de ses tests ; une archive issue d'un code dont les tests
      échouent n'a pas à exister
- [x] `codeStyle` remonté avant ses dépendants : l'initialisation des propriétés de
      premier niveau suit l'ordre du fichier
- [ ] **Conséquence à connaître** : un écart de forme sur `main` bloque tout, y compris
      l'empaquetage. C'est le modèle voulu, mais c'est une porte unique

---

## Phase 13 — Défauts trouvés à la première exécution réelle

Le 2026-09-23, première exécution depuis la migration. Quatre défauts, dont deux
graves, qu'aucun test ne pouvait attraper.

- [x] **L'autosave ne fonctionnait plus du tout.** Les réglages sont stockés en YAML et
      le lecteur ne produit que des `std::string` ; le code relisait
      `general/data_location` en `std::filesystem::path`, donc `std::any_cast` échouait
      et rendait un chemin **vide**. Aucune sauvegarde de secours n'était écrite, avec
      pour seule trace une ligne d'avertissement. Tout passe maintenant par
      `core::getDataLocation()`, qui lit une chaîne et ne rend jamais vide
- [x] **Pourquoi aucun test ne l'a vu** : le fixture de `test_Rescue.cpp` posait la
      valeur comme un `path`. Il validait donc une configuration qui n'existe jamais en
      production. Corrigé, plus deux tests sur le cas réel
- [x] **Régression de la phase 11** : le support Wayland avait basculé l'application sur
      Wayland, où les fenêtres détachées sont impossibles — or c'est précisément ce qui
      envoie l'affichage sur le vidéoprojecteur. X11 est demandé par défaut, XWayland
      compris (`MainWindow::selectPlatform`, réglage `gui/display_server`)
- [x] **Le journal s'ouvrait dans le répertoire courant** : `main` initialisait le
      logger avant `initializeUtilities`, donc `getExecPath()` était encore vide et le
      chemin absolu devenait relatif. Lancé depuis un menu, la trace du plantage était
      perdue — l'inverse de ce que S0 promettait
- [x] Le périphérique Vulkan est journalisé, avec un avertissement explicite s'il est
      logiciel : un rendu llvmpipe ne se voit sinon qu'à la lenteur, un après-midi
- [x] **Chemins d'outils périmés dans le cache CMake** : `find_program` met en cache un
      chemin absolu dans le venv Poetry, donc sous `$HOME`. Le même répertoire de build
      ouvert depuis l'IDE, un terminal et un agent CI voit trois `$HOME` différents, et
      `execute_process` sur un programme absent échoue **sans aucune sortie**.
      `cmake/Poetry.cmake` invalide les entrées mortes, en un seul endroit

**Hors application, côté poste** — la toolchain CLion « Docker Owl » ne donnait à l'uid
1001 aucun groupe supplémentaire : pas d'accès à `/dev/dri`, donc rendu llvmpipe, donc
présentation via MIT-SHM, donc `BadAccess` du serveur X. `--privileged` ne contourne pas
le contrôle de permission sur le fichier DRM. Corrigé par
`--ipc host --group-add video --group-add 990`.

---

## Protection de `main` — deux checks à exiger

**Correction d'une affirmation fausse de ma part** : j'avais dit que `main` n'avait
aucune protection de branche. C'est inexact — elle est protégée par un **ruleset**
(« main merging »), et l'ancien endpoint `/branches/main/protection` renvoie 404 pour
les rulesets, ce qui m'a trompé. Le ruleset impose déjà : pas de suppression, pas de
force-push, historique linéaire, passage par une pull request en squash uniquement.

Ce qui manque est réel, mais ne demande que **deux** noms, pas neuf, depuis que la
chaîne de dépendances existe : rien n'atteint les analyses si le style, les quatre
builds et les quatre sanitizers ne sont pas passés.

- [ ] Ajouter au ruleset « main merging » une règle `required_status_checks` avec
      exactement :
      `TeamCity / Evenement Loto / Analysis / Clang-Tidy` et
      `TeamCity / Evenement Loto / Analysis / Static Analyzer`

---

## Phase 14 — Chaîne de dépendances et portée des analyses

Demandé le 2026-09-23.

- [x] **Une seule configuration par outil** au lieu de deux. La portée n'est plus un
      paramètre de configuration mais une décision d'exécution : `ci_action.py Analysis`
      lit le numéro de pull request publié par le pont et choisit `diff` + constat
      bloquant à l'intérieur d'une PR, `full` + constat consultatif ailleurs.
      `Clang-Tidy (diff)` et `Static Analyzer (diff)` disparaissent
- [x] Chaîne : `Code Style` → les quatre builds → les quatre sanitizers → les deux
      analyses. Une dépendance en échec rend le dépendant en échec, il n'est pas
      seulement sauté
- [x] Les sanitizers attendent les deux builds **Clang**, sur les deux plateformes : ce
      sont ceux dont ils dérivent, et ils sont eux-mêmes Clang uniquement. Un Windows
      cassé arrête donc bien la chaîne
- [x] Les builds **GCC restent hors chaîne** : ils tournent sur chaque pull request et
      publient leur propre résultat, mais faire attendre quatre vagues sur un second
      compilateur allongerait chaque PR pour rien
- [x] Les **Package** ne tournent que sur `main` : `triggerOnPrReady = false`, pas de
      brouillon, et un `vcsTrigger` limité à `+:main`. Une dépendance ne tire que vers
      l'amont, donc un build Clang déclenché par une PR ne les entraîne pas
- [x] Sur une PR **brouillon**, seuls les deux builds Clang (Linux et Windows) sont
      déclenchés. `Code Style` y arrive quand même, par la dépendance, donc il n'a plus
      besoin de son propre `triggerOnPrDraft`
- [x] Conséquence voulue : seules les deux analyses sont à exiger avant un merge,
      puisqu'elles sont en fin de chaîne

## Validation manuelle — la séance de tests à faire

Tout ce qui suit demande un écran, un second écran, ou de tuer le processus. À faire en
une session, dans cet ordre : les trois premiers sont ceux qui comptent vraiment.

### 1. L'autosave écrit réellement 🔴

C'est le défaut de la phase 13, celui qui ne laissait qu'une ligne d'avertissement.

1. Régler le répertoire de données dans les préférences, puis **relancer** l'application
   (le bug n'apparaissait qu'après un rechargement des réglages depuis `config.yml`).
2. Ouvrir un événement, démarrer une partie, tirer trois numéros.
3. Vérifier que `rescue.lev` existe dans ce répertoire, horodaté à l'instant.
4. Tirer un numéro de plus, vérifier que l'horodatage bouge et que `rescue.lev.1` apparaît.

Le journal ne doit contenir **aucun** « Aucun emplacement de données configuré » ni
« Autosave failed ».

- [ ] Fait

### 2. Reprise après un arrêt brutal 🔴

1. En pleine partie, après une dizaine de numéros, `kill -9` sur le processus.
2. Relancer : la fenêtre de reprise doit proposer l'événement, avec **le bon nombre de
   numéros tirés** et une ancienneté cohérente.
3. Accepter : la partie reprend au même point.
4. Refaire l'essai en refusant : le fichier doit être archivé sous
   `rescue-<horodatage>-rescue.lev`, pas supprimé.

- [ ] Fait

### 3. Affichage sur le vidéoprojecteur 🔴

1. Second écran branché, détacher la vue d'affichage et la passer en plein écran dessus.
2. Vérifier que la grille reste lisible et que l'écran de contrôle reste utilisable.
3. Débrancher le second écran **en cours de partie**, puis le rebrancher : l'application
   ne doit ni se fermer ni perdre la partie (le chemin `OUT_OF_DATE` est traité, jamais
   vérifié en vrai).

- [ ] Fait

### 4. Le journal au bon endroit

Lancer depuis un lanceur de bureau ou un raccourci, pas depuis l'IDE, et vérifier la
ligne `Journal : '…'` : elle doit pointer à côté de l'exécutable, jamais un chemin
relatif.

- [ ] Fait

### 5. Le bon périphérique graphique

Vérifier la ligne `[vulkan] Périphérique : …` du journal. Si elle dit `llvmpipe`, le
rendu est logiciel et tiendra mal une séance — l'avertissement qui suit le dit.

- [ ] Fait

### 6. Les deux autres dialogues

Enregistrer sous, et sélectionner un dossier dans les préférences. L'ouverture est déjà
vérifiée.

- [ ] Fait

### 7. Wayland, pour mémoire

Mettre `gui/display_server: wayland` dans `config.yml`, lancer : l'application doit
démarrer et journaliser que les fenêtres détachées sont désactivées. Remettre `x11`
ensuite. C'est un test de non-régression du repli, pas un mode utilisable pour une
séance.

- [ ] Fait

### Ce qui restera non testé

L'injection d'exception dans une vue (S4) n'est pas déclenchable depuis l'interface : il
faudrait un `throw` temporaire dans une vue pour vérifier que l'application survit, puis
un `throw` permanent pour vérifier l'arrêt propre après cinq échecs. À faire avec un
correctif jetable, ou à laisser de côté.

---

## Risques ouverts

| # | Risque | Phase | Atténuation |
|---|---|---|---|
| ~~R1~~ | `compiler.cppstd` ⇒ reconstruction depuis les sources | 4 | **confirmé et assumé** : `cppstd` est obligatoire (spdlog, gtest), 7 paquets construits, cache persistant |
| ~~R2~~ | `IMGUI_API` non exporté en DLL MinGW | 6 | **confirmé puis résolu** : imgui en statique |
| ~~R3~~ | ~~clang 22 + `-Weverything`~~ | 1 | **levé** : aucun nouveau diagnostic |
| ~~R4~~ | `xorg/system` échoue faute de `-dev` X11 | 4 | **levé** via `[platform_requires]` + pont `PKG_CONFIG_PATH` |
| R5 | MinGW entièrement `--build=missing` | 6 | cache `~/.conan2` persistant, coût unique |
| ~~R6~~ | `CMakeConfigDeps` expérimental | 4 | **sans objet** : la release 0.19.0 utilise `CMakeDeps`, stable |
| R7 | Venv Poetry partagé entre images gcc et clang du même agent | 2 | même version de Python ; sinon deux venv cohabitent |
| ~~R8~~ | `copy_shared_libraries()` et cibles importées Conan | 4/9 | **résolu** : réécrite autour de `TARGET_RUNTIME_DLLS`, puis vidée de son enjeu par la phase 9. La cible importée de glfw n'exposait pas d'`IMPORTED_LOCATION` sous MinGW, donc sa DLL n'était jamais copiée (`0xc0000135` sur les tests) ; en statique il ne reste que `vulkan-1.dll`. La commande est désormais un `true` quand la liste est vide, sinon `copy_if_different` échoue faute de source |
| ~~R9~~ | `system_libs` de `xorg/system` propagés par glfw statique | 9 | **résolu** : `-Wl,--as-needed`, 68 → 10 `DT_NEEDED` |

---

## Annexes

### A. Paquets Docker — état des lieux

Présents et suffisants (dépôt `CI/DockerImages`) :

- `base/ubuntu2404.sh` : `python3`, `python3-pip`, **poetry dans `/usr/poetry`**
  (déjà sur le `PATH`), `git`, `curl`, `ca-certificates`, `libgtk-3-0t64`,
  `libvulkan1`, `libglfw3`
- `_common/builder.sh` : **cmake (Kitware ⇒ ≥ 3.24)**, `ninja-build`, `mold`,
  `ccache`, `patchelf`, **`doxygen` + `graphviz`**, `pkg-config`, `libx11-dev`,
  **`libgtk-3-dev`**, `libvulkan-dev`, `libglfw3-dev`

Vérifié en conteneur : cmake **4.3.1**, gcc **14.2.0**, clang **22.1.3** et **18.1.3**,
python **3.12.3**, poetry **2.3.4**, doxygen 1.9.8 + graphviz, et les `-dev` X11 requis
par glfw sont bien là (`libglfw3-dev` les tire). Le build Conan passe **sans modifier
l'image**.

À changer :

- [x] **Vérifié le 2026-09-23** : plus aucun paquet pip `depmanager` ni `gcovr` dans
      `builder-ubuntu2404` ni `devel-ubuntu2404`, poetry présent dans les deux
- [x] ~~phase 7 : retirer `depmanager` (et `gcovr`) du `pip install` de
      `_common/builder.sh` — plus aucun consommateur côté projet
- [ ] **optionnel, pour restaurer le WSI complet du loader Vulkan** :
      `libxcb1-dev libx11-xcb-dev libwayland-dev`, puis remettre
      `with_wsi_xcb=True` / `with_wsi_wayland=True` dans `conanfile.py`.
      Sans ça le loader embarqué ne gère que X11 — cohérent avec glfw, qui est
      lui aussi construit en X11 seul, mais c'est une réduction par rapport au
      SDK Vulkan livré par DepManager.

Inutiles : `libdbus-1-dev` (sauf `NFD_PORTAL=ON`), autotools, `perl`.

### B. Ordre d'inclusion dans `BaseConfig.cmake` (cible)

```
project()  ──► CMAKE_PROJECT_TOP_LEVEL_INCLUDES = cmake/conan_provider.cmake
               (enregistre le provider, n'exécute rien)
   │
   ├─ include(BaseConfig)
   │     ├─ Poetry.cmake      poetry sync → installe conan dans le venv, PATH mis à jour
   │     ├─ Conan.cmake       conan config install ./conan/config  (profils + remotes)
   │     ├─ UtilityFunctions.cmake
   │     ├─ DocumentationConfig  find_package(Doxygen … BYPASS_PROVIDER)
   │     ├─ CoverageConfig / Sanitizers   (find_program, non interceptés)
   │     └─ Vulkan.cmake      (définit une fonction, aucun find_package immédiat)
   │
   └─ add_subdirectory(source)
         └─ find_package(jsoncpp)  ──► ★ déclenche conan install, une seule fois
```

### C. Commandes de référence

```bash
# Configure + build
cmake --preset linux-gcc-release
cmake --build output/build/linux-gcc-release

# Tests
ctest --test-dir output/build/linux-gcc-release --output-on-failure

# Via la CI (identique en local et sur l'agent)
poetry run python3 -u ci_action.py Build linux-gcc-debug
```

---

## Phase 11 — Vulkan complet et Wayland

**Faite.** X11 reste supporté, mais Wayland devient la cible principale.

- [x] `vulkan-loader` : `with_wsi_xlib`, `with_wsi_xcb` **et** `with_wsi_wayland` à `True`
      — le loader livré annonce bien `VK_KHR_xlib_surface`, `VK_KHR_xcb_surface` et
      `VK_KHR_wayland_surface`
- [x] `glfw` : `with_x11` **et** `with_wayland` à `True` ; la plateforme est choisie à
      l'exécution. Conan construit en plus `wayland`, `xkbcommon`, `wayland-protocols`,
      `libffi`, `libxml2`, `expat`
- [x] `[platform_requires] xorg/system` **supprimé** des profils : c'était un
      contournement qui masquait aussi les `.pc` système à meson, et faisait échouer
      `xkbcommon` (`xcb-xkb >= 1.10 not found`)
- [x] **Profils séparés host/build** : `conan/config/profiles/linux-build` (gcc) est
      utilisé pour le contexte *build*. Sans cela `flex` échouait
      (« no acceptable C compiler found in $PATH ») car l'image clang n'a aucun
      compilateur C natif, et `tools.build:compiler_executables` n'atteint pas les
      recettes autotools. Bénéfice supplémentaire : les paquets d'outils sont partagés
      entre les profils gcc et clang.
- [x] `copy_shared_libraries()` réécrite : `$<TARGET_RUNTIME_DLLS>` sous Windows, rien
      sous Linux. ⚠️ **le chemin Windows n'est pas testé** (phase 6).

### Le code de l'application doit aussi être Wayland-compatible

Construire glfw avec Wayland ne suffit pas : **Wayland interdit à un client de connaître
ou de fixer la position de ses propres fenêtres**. Deux conséquences ont été traitées.

- [x] `ImGuiConfigFlags_ViewportsEnable` (`MainWindow.cpp:114`) n'est plus activé sous
      Wayland : une fenêtre ImGui détachée devient une fenêtre système qu'il faut
      positionner, ce que le protocole refuse. Dear ImGui ne supporte pas le
      multi-viewport sur Wayland.
- [x] `getMonitorsInfo()` n'appelle plus `glfwGetWindowPos()` sous Wayland (qui échoue
      avec `GLFW_FEATURE_UNAVAILABLE` et remplit le journal d'erreurs GLFW) :
      l'écran de contrôle est déduit de l'écran principal. Le comportement observable
      est le même qu'avant — c'était correct *par accident*, `windowPos` restant à
      `{0,0}` — mais c'est maintenant explicite.
- [x] `glfwGetPlatform()` est journalisé au démarrage (« Serveur d'affichage : … »)
- [x] **Tranché par l'usage** : sous Wayland les fenêtres détachées sont désactivées,
      donc la vue d'affichage ne peut pas aller sur un second écran. C'est X11 qui est
      demandé par défaut (réglage `gui/display_server`), et les fenêtres détachées y
      sortent bien — confirmé le 2026-09-23
- [x] Images reconstruites et publiées, avec une nouvelle organisation en trois
      couches : `base-ubuntu2404` (exécution), `builder-ubuntu2404` (gcc **et** clang,
      toutes les libs `-dev`), `devel-ubuntu2404` (+ debuggers). Ubuntu 22.04 et 26.04
      sont également couvertes.
- [x] `ci/PresetsParameters.json` aligné : les 7 presets Linux utilisent
      `builder-ubuntu2404`

### Rien à embarquer pour Wayland

`glfw` 3.4 charge `libwayland-client.so.0`, `libwayland-cursor.so.0`,
`libxkbcommon.so.0`, `libX11.so.6` et `libX11-xcb.so.1` par **`dlopen` au soname** :
aucune entrée `NEEDED`. Les paquets Conan `wayland`/`xkbcommon` ne servent donc qu'à la
compilation, et l'archive livrée n'a pas à les contenir — le système de l'utilisateur
les fournit, en session X11 comme en session Wayland.

### Paquets requis côté images — **présents**

Vérifié dans `builder-ubuntu2404` publiée : gcc 14.2.0, clang 22.1.8, cmake 4.4.3,
poetry 2.5.1, `lld`, `mold`, et `pkg-config` répond pour `x11`, `xcb`, **`xcb-xkb`**,
`x11-xcb`, `wayland-client`, `xkeyboard-config` et **`libdecor-0`**.

**Couche `builder`** : jeu X11/XCB complet pour `xorg/system`, `libwayland-dev`,
`libdecor-0-dev`, et **gcc à côté de clang** pour le profil de build. Le
`pip install depmanager gcovr` a disparu *(item de la phase 7)*.

**Couche `base`** : `libwayland-client0`, `libwayland-cursor0`, `libwayland-egl1`,
`libwayland-server0`, `xkb-data` et **`libdecor-0-0`** — ce dernier est indispensable,
glfw le charge par `dlopen` pour décorer ses fenêtres Wayland (sans lui, pas de barre
de titre sous GNOME).

**Validation**
- [x] `linux-gcc-debug`, `linux-gcc-release` : build + `ctest`
- [x] `linux-clang-debug`, `linux-clang-tidy`, 4 sanitizers : build + `ctest`
- [x] `linux-clang-release` : build + `cpack`, archive complète
- [x] Les trois extensions WSI présentes dans le loader livré
- [x] 45 symboles Wayland dans le `libglfw.so` construit
- [x] Démarrage réel en session Wayland vérifié le 2026-09-23 : l'application démarre,
      charge une partie et s'arrête proprement

---
---

# Partie II — Stabilité

> **Objectif** : l'application doit tenir une après-midi complète de loto (4 h et
> plus) sans le moindre crash ; et si un incident survient malgré tout, l'état
> doit être restauré en quelques secondes, sans manipulation de fichiers par
> l'utilisateur.
>
> **Indépendant de la migration Conan.** Les fichiers touchés sont
> `source/core/Event.cpp`, `source/gui/Application.cpp`,
> `source/gui/actions/FileActions.cpp`, `source/main.cpp`, `source/core/Log.cpp`
> et `source/gui/vulkan/VulkanContext.cpp` — **aucun recouvrement avec le système
> de build**. Les deux chantiers peuvent avancer en parallèle.

## Constat central

**`rescue.lev` est écrit toutes les 10 s… et n'est jamais relu.**
`grep -rn "rescue" source/` ne renvoie que l'écriture (`Application.cpp:264`).
Aucun code ne détecte sa présence au démarrage, ne la propose, ni ne la charge.
Aujourd'hui, reprendre après un crash suppose de savoir que le fichier existe, de
savoir où pointe `general/data_location`, et de l'ouvrir à la main.

**Les trois phases qui donnent la garantie** : S1 (écriture atomique),
S3 (reprise au démarrage), S2 + S4 (ne jamais crasher sur un fichier abîmé).
Le reste est du durcissement.

---

## Phase S0 — Ne plus perdre le log du crash

**Une ligne, à faire en premier** : sans ça, tout diagnostic des phases suivantes
part avec une main dans le dos.

- [x] `basic_file_sink_mt(getLogPath(), true)` → `rotating_file_sink_mt` : le journal
      n'est plus tronqué au démarrage, la trace du crash survit au redémarrage
- [x] Politique retenue : **5 Mo × 5 fichiers** (25 Mo au plus)
- [x] `spdlog::flush_every(1s)` n'était actif **qu'en debug** — activé aussi en release,
      car les dernières secondes avant un crash sont les seules qui comptent
- [x] Journaliser en en-tête de session la version, le compilateur, la plateforme, la
      version de sauvegarde et le chemin du journal (`Log::logSessionHeader`)
      *(la version et le chemin d'exécution le sont déjà)*

**Validation**
- [x] Deux démarrages consécutifs : le log du premier reste lisible (sink en ajout)
- [x] Une session longue ne remplit pas le disque (rotation)

---

## Phase S1 — Écritures atomiques et rotation

**Faite.** `source/core/AtomicFile.h/.cpp` fournit `writeFileAtomically()` : écriture
dans `<nom>.tmp`, `flush`, contrôle de `good()`, puis `std::filesystem::rename()`. La
cible n'est remplacée qu'une fois le contenu complet sur le disque.

- [x] `writeFileAtomically(path, writer, keepPrevious)`, `noexcept` — la garantie est
      réelle : le corps est isolé dans une fonction interne et le point d'entrée
      rattrape tout, puisque les appelants sont souvent sur un chemin d'arrêt
- [x] En cas d'échec : le `.tmp` est supprimé et **la cible précédente reste intacte**
- [x] Rotation à deux générations pour l'autosave (`rescue.lev` → `rescue.lev.1`),
      en « meilleur effort » : perdre l'ancienne génération n'empêche pas la nouvelle
- [x] Les trois sites d'écriture passent par cette fonction (`Application::autoSave`
      via `core::saveRescue`, `SaveFileAction`, `SaveAsFileAction`)
- [x] `FileActions` ne journalise plus « saved successfully » quand l'écriture a échoué
- [x] `SaveAsFileAction` met à jour le fichier courant (il ne le faisait pas)
- [x] La logique de répertoire de `autoSave()` ne fonctionne plus « par accident »
      sur un chemin vide
- [x] Le `.tmp` est supprimé en cas d'échec et jamais laissé derrière
      *(vérifié par `test/lib_test/test_AtomicFile.cpp`)*
      *(le filtre `lev` les exclut, à confirmer à l'usage)*

## Phase S2 — Lecture défensive du format binaire

**Faite.** `source/core/StreamRead.h` fournit `readRaw`, `readEnum`, `readLength`,
`readString` et `readVector`. Le **canal d'erreur est le flux lui-même** (`failbit`) :
aucun changement de l'interface `Serializable`, et les appelants testent `good()`.
**Le format sur disque est inchangé** (mêmes largeurs, même ordre).

- [x] État du flux vérifié après **chaque** lecture
- [x] Toutes les longueurs bornées : 1 Mio pour les chaînes et tableaux d'octets,
      65 536 pour les vecteurs d'objets
- [x] Toutes les énumérations validées par `magic_enum::enum_cast` : `Event::Status`,
      `GameRound::Type/Status`, `SubGameRound::Type/Status`
- [x] Une version de fichier supérieure met le flux en échec au lieu de laisser
      l'objet à moitié initialisé
- [x] `Event::read`, `GameRound::read` et `SubGameRound::read` réécrites
- [x] `Event::getStatusStr()` n'utilise plus de `.at()` non gardé *(fait en phase 5,
      via les tables `constexpr` de `EnumLabel.h`)*
- [x] `FileActions::LoadFileAction` : l'échec de lecture prévient désormais à l'écran,
      via `views::PopupMessage` et `Application::tell()` — un échec écrit seulement dans
      le journal est un échec que personne ne voit pendant une partie. Idem pour un
      enregistrement qui n'aboutit pas
- [x] ~~exploiter l'échec de lecture pour prévenir
      l'utilisateur *(la lecture est sûre, il reste à afficher l'erreur)*

**Validation** — `test/lib_test/test_Serialization.cpp`, 7 tests
- [x] Aller-retour complet d'un événement (parties, sous-parties, diaporama, règles)
- [x] **Troncature à chaque offset** : aucun n'est accepté, aucun crash
- [x] 64 tampons d'octets aléatoires : tous rejetés
- [x] Flux vide, version future, statut hors domaine, longueur absurde : tous rejetés
- [x] Suite verte sous gcc 14, clang 22, ASan et UBSan

## Phase S3 — Reprise après incident au démarrage

**Faite.** C'était le chaînon manquant : l'autosave existait déjà mais n'était jamais
relu. `source/core/Rescue.h/.cpp` porte la logique, testable hors interface, et
`source/gui/views/RescuePopup.h/.cpp` la fenêtre de proposition.

- [x] Détection de `rescue.lev` dans `general/data_location` au démarrage
- [x] Pertinence : statut ni `Invalid` ni `Finished`, et fichier réellement lisible
- [x] Proposition avec le nom de l'événement, le nombre de numéros déjà tirés et
      l'ancienneté en clair (« il y a 12 minutes »), pour que l'organisateur vérifie
- [x] Si `rescue.lev` est illisible, **bascule automatique sur `rescue.lev.1`**
- [x] « Ignorer » **archive** (`rescue-<horodatage>-rescue.lev`) au lieu de supprimer :
      un refus par erreur n'est pas définitif
- [x] Documenté dans `document/Utilisation.md`, section « En cas d'incident »
      *(doc utilisateur, intégrée à l'aide de l'application)*
- [x] `pruneRescueArchives()` : les 10 fichiers les plus récents sont conservés, le
      reste est supprimé. Appelé par `archiveRescue()`, donc à chaque archivage
- [x] La sauvegarde explicite **archive** le fichier de secours
      (`Application::forgetRescue`) : la partie est en sûreté dans le fichier de
      l'utilisateur, mais un archivage reste réversible là où une suppression ne l'est
      pas — et l'élagage ci-dessus borne l'accumulation

**Validation** — `test/lib_test/test_Rescue.cpp`, 6 tests
- [x] Enregistrement, détection puis rechargement d'une partie en cours
- [x] Zone vide : rien n'est proposé
- [x] Deuxième enregistrement : la génération précédente est conservée
- [x] **Génération la plus récente tronquée ⇒ bascule sur la précédente**
- [x] Un événement non repris (`Invalid`) n'est pas proposé
- [x] L'archivage conserve le fichier
- [ ] `kill -9` en pleine partie sur l'application réelle *(non vérifiable sans écran)*

## Phase S4 — Filet global contre les exceptions

**Faite.** Une exception ne peut plus ni terminer le processus en silence, ni mettre fin
à l'événement.

- [x] `try/catch` dans `main()` : type et message journalisés, `EXIT_FAILURE` renvoyé.
      Le gestionnaire de dernier recours est `noexcept`, donc rien ne peut s'échapper
      *(confirmé par `bugprone-exception-escape`, vert)*
- [x] **`try/catch` par itération** dans `Application::run()` : le corps de la frame est
      extrait dans `renderFrame()`, et une exception y est journalisée sans interrompre
      la partie. Au-delà de **5 échecs consécutifs**, `reportError()` provoque un arrêt
      propre — ce qui sauvegarde l'événement (S5) — pour ne pas boucler indéfiniment sur
      la même erreur.
- [x] Sauvegarde d'urgence : si une exception s'échappe malgré tout de `run()`,
      `main.cpp` sauvegarde **pendant que l'`Application` est encore vivante** (son
      destructeur s'exécute lors du déroulement de pile qui suit), puis relance
- [x] `Log` reste sûr au moment du `catch` de `main()` : `Log::log()` teste
      `initiated()` avant d'écrire, donc même un échec de `Log::init` ne crashe pas

**Validation**
- [x] `writeFileAtomically` avec un writer qui lance : échec propre, cible intacte
      (`test/lib_test/test_AtomicFile.cpp`, 4 tests)
- [x] Suite verte sous gcc 14, clang 22, clang-tidy et les 4 sanitizers
- [ ] Exception injectée dans une vue ⇒ l'application survit *(non automatisable en
      l'état : `Application` n'est pas instanciable sans écran ni Vulkan, c'est la
      raison pour laquelle `test/gui_test/test_Application.cpp` est commenté)*
- [ ] Exception injectée en boucle ⇒ arrêt propre après 5 échecs *(idem)*

## Phase S5 — Autosave déclenché par les événements métier

**Faite.** L'écart entre « le numéro annoncé aux joueurs » et « le numéro enregistré »
est désormais nul.

- [x] `Application::saveProgress()` (public, force la sauvegarde) appelé après chaque
      mutation : tirage, annulation de tirage, changement d'état de partie
      (`RandomPickAction`, `CancelPickAction`, `GameNextActions`)
- [x] `autoSave(bool iForce)` : le garde-fou de 10 s reste pour le rythme périodique
- [x] Sauvegarde forcée **avant toute sortie**, boucle principale comme chemin d'erreur
      ⚠️ *coché à tort dans un premier temps : le `autoSave(true)` en sortie de boucle
      n'avait jamais été appliqué (un remplacement silencieux avait échoué). En place
      depuis S4.*
- [x] `reportError()` sauvegarde, et ne le fait qu'à la première transition vers `Error`
- [x] Coût mesuré sur un build release, via le banc d'endurance : **396 144
      sauvegardes en 15 s**, soit **38 µs** par cycle tirage + écriture atomique. À 60
      images par seconde une image dure 16 667 µs, donc l'écriture synchrone coûte
      **0,23 %** d'une image. Aucune raison de la sortir de la boucle

## Phase S6 — Robustesse GPU et session longue

**Faite pour l'essentiel.** Plus aucun gel possible, et tout incident GPU laisse une
partie récupérable.

- [x] `VK_ERROR_DEVICE_LOST` traité à part, avec un message explicite
      (« La carte graphique a été réinitialisée. ») au lieu du message générique
- [x] **Sauvegarde forcée avant la sortie sur erreur** (via `reportError`, phase S5) :
      l'incident devient récupérable au redémarrage grâce à S3
- [x] **Plus d'attente infinie** : `vkWaitForFences` passe de `UINT64_MAX` à **5 s**, et
      un dépassement est traité comme un GPU bloqué (journal `critical` + sortie propre)
      au lieu d'un gel silencieux pour le reste de l'après-midi
- [x] `vkAcquireNextImageKHR` passe de `UINT64_MAX` à **2 s** ; un dépassement
      reconstruit la swapchain, ce qui est la récupération normale
- [x] Retour de `vkEnumerateInstanceExtensionProperties` (premier appel) vérifié
- [ ] Recréer device + swapchain sur `DEVICE_LOST` *(récupération dans le processus ;
      demanderait de recharger toutes les textures — laissé ouvert)*
- [ ] Débranchement/rebranchement de l'écran secondaire en cours de partie
      *(le chemin `OUT_OF_DATE` est traité, reste à valider en vrai)*
- [x] Banc d'endurance : `test/lib_test/test_Endurance.cpp`, désactivé par défaut
      (`DISABLED_`, durée par `EVL_ENDURANCE_SECONDS`). Il rejoue l'après-midi qui
      compte — un numéro tiré, la partie sauvegardée atomiquement, le fichier relu — et
      surveille le RSS et les descripteurs
- [x] Mesuré : **258 139 tirages-sauvegardes et 5 162 relectures en 20 s**, RSS
      10 400 → 10 736 KiB (plateau), descripteurs 5 → 5. Soit environ 12 900
      sauvegardes/seconde soit, en 20 secondes, plusieurs centaines de fois la charge
      d'un après-midi réel
- [ ] Run de 4 h en horloge murale — **décidé le 2026-09-23 : pas dans l'environnement
      de développement.** La charge est déjà couverte plusieurs centaines de fois par le
      banc ci-dessus ; ce qui reste à observer est lié au temps lui-même (rotation du
      journal, dérive d'horloge) et se voit mieux sur la machine qui animera l'après-midi.
      Le banc est là pour ça :
      `EVL_ENDURANCE_SECONDS=14400 evl_lib_test_unit_test --gtest_also_run_disabled_tests
      --gtest_filter='*Endurance*'`
- [x] Session d'endurance sous les deux sanitizers. **LeakSanitizer propre** :
      13 108 → 13 608 KiB sur 421 233 cycles. AddressSanitizer ne signale **aucune
      fuite** non plus, mais son RSS grimpe à 355 MiB : sa quarantaine retient les blocs
      libérés par construction, donc la mesure mémoire n'y veut rien dire. L'assertion
      RSS est exemptée sous ASan, et le test le documente

## Phase S7 — Format de fichier portable

**Faite**, version de sauvegarde **7**. Un fichier s'ouvre sur un nombre magique `EVL1`
suivi de la version, et se termine par un CRC-32 vérifié **avant** qu'un seul champ ne
soit interprété — c'est ce qui permet d'écarter un `rescue.lev` tronqué au profit de la
génération précédente au lieu d'en charger la moitié.

- [x] Toutes les longueurs en `uint64_t` (`writeLength`, `readLength`)
- [x] Les dates en `int64_t` de nanosecondes depuis l'epoch, unité **nommée** au lieu
      du dump brut d'un `time_point` dont la représentation appartient à la
      bibliothèque standard
- [x] Les énumérations : **rien à faire**, toutes les énumérations sérialisées déclarent
      déjà `: uint8_t`, donc une largeur fixe et explicite
- [x] `getSaveVersion()` passe à 7, la lecture des versions antérieures est conservée
- [x] Nombre magique en tête, pour rejeter tôt un fichier qui n'est pas un `.lev`
- [x] Somme de contrôle CRC-32 en queue, vérifiée avant la lecture du corps
- [x] **Bug corrigé au passage** : `LoadFileAction` annonçait « loaded successfully »
      sans jamais regarder l'état du stream. Un fichier corrompu laissait
      l'application avec un événement à moitié lu. La lecture se fait maintenant dans
      un candidat, qui ne remplace l'événement courant qu'une fois complète.

**Validation**
- [x] Aller-retour, troncature à chaque offset, octets aléatoires, version future,
      énumérateur hors bornes, longueur absurde — 12 tests
- [x] Un octet modifié est détecté par la somme de contrôle
- [x] Un fichier de version 6 sans cadre se relit toujours
- [x] Un fichier qui n'est pas un `.lev` est rejeté sur son nombre magique
- [x] Réglé plus fort qu'attendu : la sérialisation est **déterministe** (test), donc
      `test/lib_test/reference-v7.lev` est figé dans le dépôt et comparé **octet pour
      octet**. Chaque toolchain de la CI — clang, MinGW gcc, MinGW clang, et un arm64 le
      jour où il y en aura — vérifie les mêmes octets, sans rien à orchestrer. Vérifié
      identique sous gcc et clang x64

### La version 6 était ambiguë — corrigé

Découvert en relisant les `.lev` livrés : trois des quatre ne se lisaient plus. Les
énumérations avaient été passées à `: uint8_t` **sans incrémenter `getSaveVersion()`**,
si bien que deux mises en page portaient le numéro 6. Preuve à l'octet sur
`data/test_sou.lev` :

```
02 00 00 00  0f 00 00 00 00 00 00 00  "Sou des écoles"
^^ status sur 4 octets              ^^ longueur sur 8 octets
```

alors que `data/super_loto.lev`, également version 6, écrit son status sur **1** octet.

- [x] `ReadContext` remplace le `int iFileVersion` de `Serializable::read` : la version
      seule ne suffisait pas à décrire le fichier parcouru
- [x] `readEnum` sait lire une énumération sur quatre octets, et **borne la valeur**
      avant conversion — un champ de quatre octets portant ce que le type sous-jacent
      ne peut pas représenter n'est pas une mise en page ancienne, c'est une corruption
- [x] `Event::read` tente la lecture en étroit puis, pour une version ≤ 6, retente en
      large. La discrimination est fiable sans deviner : lu en étroit, le premier champ
      de `test_sou.lev` donne une longueur de 251 658 240 que la borne existante rejette
- [x] Les quatre fichiers livrés se relisent, **et leur contenu est vérifié** : nom,
      organisateur et nombre de parties sont désormais assertés fichier par fichier.
      Un mauvais décodage peut parcourir un fichier jusqu'au bout et rendre n'importe
      quoi — c'est précisément ce qu'ouvrir un vieil événement ne doit pas faire
- [x] Les marqueurs `//----UNCOVER----` des branches « version < 4 » retirés : ces
      branches sont maintenant réellement exercées par les fichiers livrés

## Phase S8 — Qualité et cohérence

🟡 **P2.** Rien de bloquant, mais autant le traiter en passant sur les fichiers.

- [x] Les `path::string()` temporaires dans la boucle d'écriture : disparus avec
      `writeString` (phase S7), un seul appel par champ
- [x] Les boucles octet par octet de `read`/`write` : **zéro restante**, remplacées par
      les écritures en bloc de `StreamWrite.h` (phase S7)
- [x] `stop_game` n'est activable qu'au statut `Finished` : **voulu**, tranché le
      2026-09-23 — on n'interrompt pas une partie en cours. La règle est désormais écrite
      à l'endroit du code, pour que personne ne la « corrige » en croyant à un oubli.
- [x] La logique `if (!exists(...) && !empty()) … else if (!is_directory(...))` : le
      bloc entier a été remplacé par `core::saveRescue()` en S1/S3, qui traite le
      dossier vide et le non-dossier explicitement
- [x] Couverture : périmètre resserré à ce qui ne peut pas tourner sans fenêtre ni GPU
      (`.*test.*|.*MainWindow.*|.*gui/vulkan.*|.*third_party.*`). `Application`,
      `gui/actions`, `gui/views` et `gui/utils` sont désormais **mesurés**, et le constat
      est net : `FileActions.cpp`, `GameActions.cpp` et `Application.cpp` sont à **0 %**.
      Un chiffre nul qu'on voit vaut mieux qu'une absence de chiffre. La logique critique
      elle-même a migré dans `core/` en S1–S3 et y est couverte (`Rescue.cpp` 89 % de
      lignes)
- [x] **Bug trouvé en mesurant** : `ci/actions/coverage.py` lançait `gcovr` sur `.`,
      donc sur **tous** les répertoires de build. Avec deux presets instrumentés, gcovr
      mélangeait les profils gcc et clang et échouait sur le conflit de version
      (`'B11*'` contre `'B42*'`). Invisible en CI, où il n'y a qu'un build par agent.
      Pointé sur le répertoire du preset

---

## Traçabilité des constats

| Constat | Gravité | Emplacement | Phase |
|---|---|---|---|
| `rescue.lev` jamais relu, aucune restauration | 🔴 | `Application.cpp:264` (seule occurrence) | S3 |
| Écritures non atomiques (3 sites) | 🔴 | `Application.cpp:265`, `FileActions.cpp:65`, `:83` | S1 |
| Aucun `try/catch` de haut niveau | 🔴 | `main.cpp`, `Application.cpp:110` | S4 |
| `read()` sans contrôle de flux ni borne | 🔴 | `Event.cpp:46-48` | S2 |
| `m_status` lu sans validation + `.at()` non gardé ⇒ crash en boucle | 🔴 | `Event.cpp:43`, `Event.cpp:32` | S2 |
| `is_open()` absent à la lecture | 🔴 | `FileActions.cpp:39` | S2 |
| « saved successfully » journalisé même en cas d'échec | 🔴 | `FileActions.cpp:67`, `:85` | S1 |
| `VK_ERROR_DEVICE_LOST` non traité, sortie sans sauvegarde | 🟠 | `VulkanContext.cpp:480` | S6 |
| Attente GPU infinie (`UINT64_MAX`) | 🟠 | `VulkanContext.cpp:~505` | S6 |
| Log tronqué à chaque démarrage | 🟠 | `Log.cpp:63` | S0 |
| Autosave uniquement périodique (fenêtre de 10 s) | 🟠 | `Application.cpp:255` | S5 |
| Format binaire non portable | 🟠 | `Event.cpp` write/read | S7 |
| `path::string()` dans une boucle ⇒ O(n²) | 🟡 | `Event.cpp:99`, `:105` | S8 |
| `stop_game` activé seulement si `Finished` | 🟡 | `Application.cpp:183` | S8 |
| Aucun test sur fichier corrompu | 🟡 | `test/lib_test/` | S2 |
| Couverture exclut l'autosave et les sauvegardes | 🟡 | `gcovr.cfg` | S8 |

## Points déjà solides (à ne pas casser)

- **`RandomNumberGenerator`** : pool `m_remaining`, tirage par index, retrait en O(1)
  par swap-and-pop, sentinelle `255` quand le pool est vide, `popNum()` pour annuler.
  **Pas de boucle de rejet** ⇒ aucun risque de blocage quand presque tous les numéros
  sont sortis, qui est le piège classique de ce genre de code.
- `LogBuffer` plafonné à 1000 entrées et `spdlog::flush_every(1s)` : pas de fuite sur
  4 h, et les logs atteignent le disque avant un crash.
- Swapchain : `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR` correctement traités
  avec reconstruction (`VulkanContext.cpp:498`, `:567`).
- Mise en cache des pointeurs de vues et d'actions (`Application.cpp:89-95`) pour
  éviter des recherches O(n) par frame.
- `Settings` a un parsing défensif en cascade (`Settings.cpp:39-72`) : c'est
  exactement le modèle qui manque à `Event::read()`.
