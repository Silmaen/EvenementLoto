# TODO — Migration technique et stabilité

**Partie I** — migration DepManager → Conan, CI TeamCity en sources, retrait de Copilot.
**Partie II** — durcissement de la stabilité (4 h sans crash, reprise immédiate sur incident).
Les deux parties sont **indépendantes** et peuvent avancer en parallèle.

> Document de travail vivant. Chaque case cochée = fait et validé.
> Les phases sont ordonnées : **ne pas sauter une phase**, chacune isole une cause
> de panne. Les phases 0 à 2 sont indépendantes de Conan.

**État global** : 🟩 phases 0, 1 et 2 faites
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
12. [Phase 9 — Tout en statique](#phase-9--tout-en-statique-optionnel)
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
| D2 | Intégration | `cmake-conan` **release 0.19.x**, fichier `conan_provider.cmake` **copié** dans `cmake/` (pas de clone, pas de `develop2`) → pas de réseau au configure, version visible dans le diff. | 2026-09-21 |
| D3 | Profils Conan | Écrits à la main et versionnés. Pas d'autodétection (elle ne couvre que Win+MSVC / Linux+gcc / Apple+clang). | 2026-09-21 |
| D4 | Doxygen | **Reste une dépendance externe** de l'image Docker. Pas de `tool_requires`. | 2026-09-21 |
| D5 | Python | **Poetry gère les venv ET les dépendances**, conan inclus. Suppression du bricolage `.env` / `VENV_PATH`. | 2026-09-21 |
| D6 | nfd | **Recette Conan locale** pour démarrer (`local-recipes-index`), puis retrait complet de la dépendance en phase 10. | 2026-09-21 |
| D7 | Liaison | Phase 4 **reproduit à l'identique** le découpage shared/static de `depmanager.yml`. La bascule tout-statique est une phase séparée (9). | 2026-09-21 |
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
- Le provider exige le générateur **`CMakeConfigDeps`** dans le conanfile
  → **Conan ≥ 2.25** (expérimental). Dernière version publiée : **2.32.0**.
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
- [ ] **Rotation des secrets** exportés en clair ou en `zxx` (brouillage réversible) :
  - [ ] clé privée SSH `github connexion` (était en clair dans le zip)
  - [ ] clé privée + client secret + webhook secret de la GitHub App « Owl »
  - [ ] `github_access_token` (paramètre racine + celui du commit-status-publisher)
  - [ ] `deploy_passwd`, `remote_passwd`
- [ ] Remonter la clé SSH au projet racine TeamCity *(décidé : cohérent avec D8,
      le root reste géré par l'UI)*
- [x] Supprimer `.github/copilot-instructions.md` *(contenu déjà couvert intégralement
      par `CLAUDE.md` : tabulations, commentaires en anglais, préfixes `m_`/`i`/`o`/`io`,
      trailing return types, `log_error`/`log_warn`/`log_info`)*
- [x] Supprimer les `.idea/copilot.data.migration.*.xml` en local
      *(4 fichiers, déjà non suivis : `.idea/.gitignore` contient `copilot.*`)*
- [ ] Désactiver Copilot côté GitHub (Settings → Copilot / Code security) — **manuel**
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
- [x] `ci/PresetsParameters.json` : `builder-clang18-ubuntu2404` →
      `builder-clang-llvm22-ubuntu2404` (6 occurrences : `linux-clang-debug`,
      `linux-clang-tidy`, 4 sanitizers)
      ⚠️ **l'image `clang18` n'est plus générée** par `generator.py` du dépôt DockerImages
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
- [ ] Doc développeur : noter dans le dépôt le nouveau flux de build
      (CMake pilote Conan) une fois la phase 4 passée

> **Régression trouvée et corrigée** : `find_package(VulkanHeaders)` était appelé mais
> sa cible **jamais liée** — le chemin d'en-têtes arrivait par accident via
> `VulkanUtilityLibraries`. En retirant celui-ci, la compilation est tombée sur le
> Vulkan **système** de l'image (1.3), d'où `VK_API_VERSION_1_4 was not declared`.
> `Vulkan::Headers` est désormais lié explicitement. À reproduire en phase 4.

**Validation**
- [x] `linux-gcc-debug` : configure + build + `ctest` OK
- [x] `linux-clang-debug` : configure + build + `ctest` OK (clang 22 **et** clang 18)
- [ ] L'application démarre et affiche une partie

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
- [ ] Le venv survit à deux builds TeamCity consécutifs *(non testable en local)*

---

## Phase 3 — Recette locale nfd

- [ ] Créer `conan/local-recipes/nfd/config.yml` (versions `1.4.0`, `1.2.1`)
- [ ] Créer `conan/local-recipes/nfd/all/conanfile.py` :
  - [ ] `name = "nfd"`, source depuis `github.com/btzy/nativefiledialog-extended`
  - [ ] `cmake_file_name = "nfd"`, `cmake_target_name = "nfd::nfd"`
        → **`source/gui/CMakeLists.txt:37-39` reste inchangé**
  - [ ] options : `portal` (défaut OFF → GTK3), `x11` (ON), `wayland` (défaut **OFF**)
  - [ ] Linux : `requires` GTK3 ou dbus selon `portal` — préférer les paquets *system*
        pour ne pas empaqueter GTK via Conan
  - [ ] Windows : `system_libs = ["ole32", "uuid", "shell32"]`
  - [ ] `NFD_BUILD_TESTS=OFF`, `NFD_INSTALL=ON`
- [ ] Créer `conan/local-recipes/nfd/all/conandata.yml` (URL + sha256 de v1.4.0)
- [ ] Déclarer le remote `local-recipes-index` dans `conan/config/remotes.json`

**Validation**
- [ ] `conan create` OK sous les 4 profils (linux-gcc14, linux-clang18/22,
      windows-mingw-gcc, windows-mingw-clang)
- [ ] Les dialogues s'ouvrent réellement (ouvrir, enregistrer, sélectionner un dossier)
- [ ] Les suppressions de `lsan_suppressions.txt` sont toujours pertinentes
      (le backend GTK3 passe aussi par dbus en interne)

---

## Phase 4 — Bascule Conan (Linux)

**Le cœur de la migration.** Découpage shared/static **identique** à
`depmanager.yml` (D7) : on ne change qu'une variable à la fois.

### Fichiers à créer

- [ ] `cmake/conan_provider.cmake` — copie depuis la **release 0.19.x** de cmake-conan
      (noter la version exacte en commentaire d'en-tête)
- [ ] `conanfile.py` (racine) :
  - [ ] `generators = "CMakeConfigDeps"` *(exigé par le provider)*
  - [ ] `requirements()` : les 11 paquets du tableau ci-dessus
  - [ ] `build_requirements()` : `self.test_requires("gtest/1.17.0")`
  - [ ] **pas de `layout()`** → dossier de générateurs = `${CMAKE_BINARY_DIR}/conan`
  - [ ] `generate()` : copier `imgui_impl_glfw.*` et `imgui_impl_vulkan.*` depuis
        `res/bindings/` du paquet imgui vers
        `${generators_folder}/imgui_bindings/backends/`
        → le sous-dossier `backends/` préserve les
        `#include <backends/imgui_impl_*.h>` de `MainWindow.cpp:19-20` et
        `VulkanContext.cpp:15`
  - [ ] options shared : `imgui`, `glfw`, `spdlog` en `shared=True` ; le reste statique
- [ ] `conan/config/global.conf`
- [ ] `conan/config/remotes.json` (conancenter + le remote local de la phase 3)
- [ ] `conan/config/profiles/linux-gcc14`
- [ ] `conan/config/profiles/linux-clang` (clang 22)
- [ ] `conan/config/profiles/windows-mingw-gcc`
- [ ] `conan/config/profiles/windows-mingw-clang`
      ⚠️ **NE PAS mettre `compiler.cppstd` dans les profils** : il entre dans le
      package ID et ferait rater tous les binaires précompilés de ConanCenter.
      Le C++23 du projet vient de `CMAKE_CXX_STANDARD 23` et ne concerne que nos cibles.
- [ ] `cmake/Conan.cmake` (remplace `cmake/Depmanager.cmake`) :
  - [ ] `conan config install ${PROJECT_SOURCE_DIR}/conan/config` (idempotent)
  - [ ] vérification `conan --version` ≥ 2.25
  - [ ] log du profil retenu et du mode (téléchargé / construit)
  - [ ] fonction `target_link_imgui_backends()` : lib statique bâtie depuis
        `${CMAKE_BINARY_DIR}/conan/imgui_bindings/backends/*.cpp`, dossier exposé en include

### Fichiers à modifier

- [ ] `CMakeLists.txt` ou presets : `CMAKE_PROJECT_TOP_LEVEL_INCLUDES` →
      `cmake/conan_provider.cmake`
- [ ] `cmake/BaseConfig.cmake:9` : `include(Depmanager)` → `include(Conan)`,
      **placé après Poetry et avant tout `find_package()`**
- [ ] `cmake/DocumentationConfig.cmake:4` : `find_package(Doxygen REQUIRED dot BYPASS_PROVIDER)`
      *(sinon le provider déclenche `conan install` trop tôt)*
- [ ] `cmake/CMakePresetsLinux.json` : `CONAN_HOST_PROFILE` par preset
- [ ] `source/core/CMakeLists.txt:31` : `jsoncpp_static` → `JsonCpp::JsonCpp`
- [ ] `source/gui/CMakeLists.txt:29-31` : `find_package(stb_image)` → `find_package(stb)`,
      `stb_image::stb_image` → `stb::stb`
- [ ] `source/gui/CMakeLists.txt:33-35` : `find_package(NanoSVG)` → `find_package(nanosvg)`,
      `NanoSVG::nanosvg` → `nanosvg::nanosvg`
- [ ] `source/gui/CMakeLists.txt` : appel à `target_link_imgui_backends()`
- [ ] `cmake/UtilityFunctions.cmake` : vérifier que `copy_shared_libraries()` fonctionne
      toujours avec les cibles importées de Conan (`LOCATION` sur une cible importée)

**Validation**
- [ ] `linux-gcc-debug` et `linux-clang-debug` : configure + build + `ctest`
- [ ] **Vérifier dans le log si Conan télécharge ou reconstruit** — si tout se
      reconstruit, c'est le symptôme d'un `cppstd` parasite dans le profil
- [ ] Les en-têtes tiers ne déclenchent pas `-Weverything` (cibles `IMPORTED` ⇒ `SYSTEM`)
- [ ] `linux-gcc-release` / `linux-clang-release` : build + `cpack` + l'archive
      contient bien les `.so` attendus
- [ ] L'application démarre, charge/enregistre une partie, affiche les images et la doc
- [ ] Un `cmake --preset` sur un cache Conan vide fonctionne (bootstrap complet)

---

## Phase 5 — Presets qualité

- [ ] `linux-clang-tidy` : configure + build
- [ ] `linux-sanitizer-address` : build + `ctest`
- [ ] `linux-sanitizer-leak` : build + `ctest`, `lsan_suppressions.txt` toujours suffisant
- [ ] `linux-sanitizer-thread` : build + `ctest`
      *(les dépendances Conan ne sont pas instrumentées — acceptable, à documenter)*
- [ ] `linux-sanitizer-undefined-behavior` : build + `ctest`
- [ ] Vérifier que clang-tidy n'analyse pas les sources tierces
      (`.clang-tidy` + en-têtes `SYSTEM`)

---

## Phase 6 — MinGW

⚠️ **ConanCenter ne publie pas de binaires pour MinGW** → tout en `--build=missing`.
Coût unique grâce au cache persistant.

- [ ] `cmake/CMakePresetsMinGW.json` : `CONAN_HOST_PROFILE` par preset
- [ ] `windows-gcc-debug` : configure + build + `ctest`
- [ ] `windows-clang-debug` : configure + build + `ctest`
- [ ] `windows-gcc-release` / `windows-clang-release` : build + `cpack`
- [ ] ⚠️ **`IMGUI_API` en DLL sous MinGW** : vérifier que la recette CCI d'imgui en
      `shared=True` exporte correctement les symboles. Si non → imgui en statique
      (voir phase 9, qui supprime le problème : les backends étant compilés dans `_ui`,
      tout reste du même côté de la frontière)
- [ ] Vérifier que les DLL MinGW (`libgcc_s_seh-1`, `libstdc++-6`, `libwinpthread-1`)
      sont toujours copiées (`cmake/BaseConfig.cmake`, cible `_SuperBase`)
- [ ] `install(CODE …)` avec `GET_RUNTIME_DEPENDENCIES` : vérifier la résolution des
      DLL issues du cache Conan (`source/CMakeLists.txt`)

---

## Phase 7 — Suppression de DepManager

- [ ] Supprimer `cmake/Depmanager.cmake`
- [ ] Supprimer `depmanager.yml`
- [ ] `pyproject.toml` : retirer `depmanager = "^0.5.1"`
- [ ] `poetry.lock` : régénérer
- [ ] TeamCity : retirer `poetry run dmgr remote add …` du runner « Tool Dependencies »
      (le runner ne garde que `PythonRequirements` + `DefineVariables`)
- [ ] TeamCity : supprimer les paramètres racine `remote_url`, `remote_login`, `remote_passwd`
- [ ] **Dépôt `CI/DockerImages`** : retirer `depmanager` du
      `pip install --break-system-packages … depmanager gcovr` de
      `ci_images/install/_common/builder.sh` (et `gcovr`, désormais fourni par `poetry.lock`)
- [ ] Reconstruire et publier `builder-gcc14-ubuntu2404` et `builder-clang-llvm22-ubuntu2404`
- [ ] Mettre à jour `CLAUDE.md` (sections « Dependency management », « External
      Dependencies », « Python Dependencies », « Build System »)
- [ ] Mettre à jour `README.md`

---

## Phase 8 — CI TeamCity en Kotlin DSL

**Méthode** : ne pas écrire `.teamcity/` à la main. Dans l'UI :
*Project Settings → Versioned Settings → Synchronization enabled, format Kotlin*.
TeamCity génère `pom.xml` + `settings.kts` avec la bonne version de DSL pour le
serveur (**2026.2**) et remplace chaque secret par un jeton `credentialsJSON:<uuid>`.

- [ ] Activer les Versioned Settings au niveau du projet *Evenement Loto*
- [ ] Récupérer le `.teamcity/` généré, **vérifier qu'aucun secret n'est en clair**
- [ ] Refactoriser `settings.kts` : une `data class` + une liste des cibles, une
      boucle qui produit les 9 `BuildType` depuis le template partagé
- [ ] Vérifier que `ci/PresetsParameters.json` reste la **source de vérité unique**
      pour `run_tests` / `run_coverage` / `run_deploy` / `run_documentation` /
      `release_preset` / `docker_image` (poussés à l'exécution par l'action `DefineVariables`)
- [ ] Ne **pas** dupliquer cette table dans le DSL
- [ ] Vérifier que les 11 runners du template « Global Build » sont fidèlement reproduits,
      conditions incluses (`run_tests`, `run_coverage`, `release_preset != ""`)
- [ ] Vérifier les extensions : commit-status-publisher GitHub, xml-report-plugin
      (gtest, `output/build/**/test/*_Report.xml`), perfmon, InvestigationsAutoAssigner
- [ ] Vérifier les onglets de rapport (Coverage.zip, Documentation.zip, static-analysis.zip)
- [ ] Comparer un build avant/après sur la même révision

---

## Phase 9 — Tout en statique (optionnel)

**Séparée volontairement de la phase 4** : ne pas changer deux variables à la fois.

- [ ] `conanfile.py` : `shared=False` partout sauf `vulkan-loader`
      (`package_type = "shared-library"` par nature)
- [ ] Garde-fou dans `cmake/Conan.cmake` : si `BUILD_SHARED_LIBS` est activé,
      `imgui`/`glfw`/`spdlog` **doivent** repasser en `shared=True`
      *(le risque d'état global dupliqué n'existe que si `_lib`/`_ui` deviennent partagées)*
- [ ] Simplifier `copy_shared_libraries()` (`cmake/UtilityFunctions.cmake:17`)
- [ ] Simplifier les blocs `GET_RUNTIME_DEPENDENCIES` (`source/CMakeLists.txt`)
- [ ] Revalider l'install et CPack sur les 4 plateformes
- [ ] Vérifier la taille du binaire et le temps de démarrage

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

## Risques ouverts

| # | Risque | Phase | Atténuation |
|---|---|---|---|
| R1 | `compiler.cppstd` dans un profil ⇒ tout se reconstruit | 4 | ne pas le déclarer ; vérifier au log |
| R2 | `IMGUI_API` non exporté en DLL MinGW | 6 | passer imgui en statique (phase 9) |
| ~~R3~~ | ~~clang 22 + `-Weverything`~~ | 1 | **levé** : aucun nouveau diagnostic |
| R4 | `xorg/system` échoue faute de `-dev` X11 | 4 | l'erreur nomme le paquet ; `libglfw3-dev` devrait suffire |
| R5 | MinGW entièrement `--build=missing` | 6 | cache `~/.conan2` persistant, coût unique |
| R6 | `CMakeConfigDeps` expérimental (« subject to breaking changes ») | 4 | épingler la version de Conan dans `poetry.lock` |
| R7 | Venv Poetry partagé entre images gcc et clang du même agent | 2 | même version de Python ; sinon deux venv cohabitent |
| R8 | `copy_shared_libraries()` et `LOCATION` sur cibles importées Conan | 4 | à tester explicitement |

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

À changer :

- [ ] phase 7 : retirer `depmanager` (et `gcovr`) du `pip install`
- [ ] à vérifier : jeu complet de `-dev` X11 pour `xorg/system`
      (`apt-cache depends libglfw3-dev`) → sinon `libxrandr-dev libxinerama-dev
      libxcursor-dev libxi-dev libgl-dev`
- [ ] si `NFD_WAYLAND=ON` : `libwayland-dev` — sinon forcer `OFF`

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

- [ ] `source/core/Log.cpp:63` : `basic_file_sink_mt(getLogPath(), true)` — le
      second paramètre **tronque le fichier à chaque démarrage**. Après un crash,
      le premier réflexe de l'utilisateur est de relancer, ce qui détruit la seule
      trace. Passer à `rotating_file_sink_mt` (ou conserver N sessions horodatées).
- [ ] Choisir la politique : nombre de fichiers conservés et taille max
      *(une session de 4 h en `Info` reste petite ; en `Trace` c'est autre chose)*
- [ ] Journaliser en en-tête de session la version, le compilateur, la plateforme
      et le chemin de `data_location` *(déjà partiellement fait dans `main.cpp`)*

**Validation**
- [ ] Deux démarrages consécutifs : le log du premier est toujours lisible
- [ ] Une session longue ne remplit pas le disque

---

## Phase S1 — Écritures atomiques et rotation

🔴 **P0.** Aujourd'hui les trois chemins d'écriture ouvrent directement le fichier
de destination :

| Site | Fichier écrit |
|---|---|
| `Application.cpp:265` (`autoSave`) | `rescue.lev` |
| `FileActions.cpp:65` (`SaveFileAction`) | le `.lev` de l'utilisateur |
| `FileActions.cpp:83` (`SaveAsFileAction`) | le `.lev` de l'utilisateur |

Un crash — ou une coupure de courant, réaliste dans une salle des fêtes —
**pendant** l'écriture laisse un fichier tronqué. Le fichier de secours est détruit
à l'instant précis où il devient utile.

- [ ] Écrire une fonction utilitaire unique `writeAtomic(path, writer)` :
  - [ ] écrire dans `<path>.tmp`
  - [ ] `flush()` puis **vérifier `good()`** puis `close()`
  - [ ] en cas d'échec : supprimer le `.tmp`, journaliser, **ne pas toucher** à la cible
  - [ ] sinon `std::filesystem::rename()` (atomique sur le même système de fichiers)
- [ ] Rotation à deux générations pour l'autosave : `rescue.lev` → `rescue.lev.1`
      avant chaque bascule *(si la dernière sauvegarde est illisible, la précédente
      reste exploitable)*
- [ ] Router les trois sites d'écriture vers cette fonction
- [ ] `Event::write()` doit signaler l'échec *(aujourd'hui `void`)*
- [ ] `FileActions.cpp:67` et `:85` : `log_info("File '{}' saved successfully.")` est
      émis **même si l'écriture a échoué** → conditionner au succès
      *(des logs qui mentent pendant un incident, c'est le pire moment)*
- [ ] `Application.cpp:262` : `create_directories()` peut lancer
      `filesystem_error` si les droits manquent → traiter *(voir aussi S4)*

**Validation**
- [ ] Test : interrompre l'écriture (writer qui lance au milieu) ⇒ la cible
      précédente est **intacte**
- [ ] Test : destination en lecture seule ⇒ message d'erreur, pas de crash, pas de
      fichier corrompu
- [ ] Vérifier que `.tmp` et `rescue.lev.1` ne polluent pas la liste du sélecteur de fichiers

---

## Phase S2 — Lecture défensive du format binaire

🔴 **P0.** `Event::read()` fait confiance au contenu du fichier :

```cpp
// Event.cpp:46-48
iBs.read(reinterpret_cast<char*>(&l), sizeof(l));
m_organizerName.resize(l);          // l vient du fichier, aucune borne
```

L'état du flux n'est **jamais** vérifié et aucune taille n'est bornée : un fichier
tronqué donne un `size_type` arbitraire → `resize()` lance `length_error` /
`bad_alloc` → `std::terminate` (voir S4).

Pire, `m_status` est lu en brut (`Event.cpp:43`) sans validation. Un octet corrompu
produit un statut hors domaine, et `Event::getStatusStr()` (`Event.cpp:32`) utilise
`.at()` **sans garde** — alors que `SubGameRound::getStatusStr()` fait correctement
`if (contains(...))` (`SubGameRound.cpp:37` et `:44`). Le statut étant affiché dans
la barre d'état à chaque frame, le résultat est un **crash en boucle au
redémarrage** : l'application ne s'ouvre plus du tout.

- [ ] Vérifier `iBs.good()` après chaque lecture, et sortir proprement sinon
- [ ] Borner toutes les longueurs avant `resize()`
      *(un nom d'organisateur ne fait pas 4 Go)*
- [ ] Valider toutes les énumérations lues via `magic_enum::enum_cast`
      *(déjà une dépendance, déjà utilisé dans `main.cpp`)* : `Event::Status`,
      `GameRound::Type`, `SubGameRound::Type`, `SubGameRound::Status`
- [ ] Garder `Event::getStatusStr()` comme `SubGameRound::getStatusStr()`
- [ ] Faire remonter l'échec : `Event::read()` retourne `void`, donc l'appelant ne
      peut pas savoir *(→ statut de retour ou exception typée)*
- [ ] `FileActions.cpp:39` : ajouter le contrôle `is_open()` avant `read()`
      *(sur un flux invalide, chaque lecture échoue en silence, les champs gardent
      leur valeur précédente, et on obtient un `Event` incohérent à moitié écrasé)*
- [ ] `Event.cpp:38` : en cas de version de fichier supérieure, on `return` en
      laissant l'objet à moitié initialisé sans en informer l'appelant → traiter
      comme un échec de chargement

**Validation** *(le test le plus rentable de tout ce document)*
- [ ] Test paramétré : tronquer un `.lev` valide **à chaque offset** ⇒ aucun crash,
      un message d'erreur, et l'état courant préservé
- [ ] Test : longueurs absurdes injectées ⇒ refus propre
- [ ] Test : statut / type hors domaine ⇒ refus propre
- [ ] Test : fichier vide, fichier de 0 octet, fichier de version future
- [ ] Test : fichier rempli d'octets aléatoires

---

## Phase S3 — Reprise après incident au démarrage

🔴 **P0.** C'est le chaînon manquant qui transforme l'autosave existant en vraie
reprise sur incident. **Dépend de S1 et S2** (ne proposer une reprise que si on
sait écrire un fichier fiable et lire un fichier suspect sans crasher).

- [ ] Au démarrage, détecter `rescue.lev` dans `general/data_location`
- [ ] Décider s'il est « pertinent » : statut ≠ `Invalid` / `Finished`, et
      horodatage plus récent que le `.lev` courant
- [ ] Proposer à l'utilisateur : « Une partie interrompue a été détectée
      (il y a *N* minutes) — reprendre ? » avec le nom de l'événement et le nombre
      de tirages effectués, pour qu'il puisse juger
- [ ] Si la lecture de `rescue.lev` échoue, **retenter avec `rescue.lev.1`** (S1)
- [ ] Après une reprise acceptée : conserver le fichier jusqu'à la première
      sauvegarde explicite *(ne pas supprimer le filet trop tôt)*
- [ ] Après une reprise refusée : archiver plutôt que supprimer
      (`rescue-<horodatage>.lev`) — un refus par erreur ne doit pas être définitif
- [ ] Nettoyer les archives au-delà de N
- [ ] Documenter la procédure dans `document/Utilisation.md`

**Validation**
- [ ] `kill -9` en pleine partie, relance ⇒ la proposition apparaît et la reprise
      restitue le bon nombre de tirages
- [ ] `rescue.lev` volontairement corrompu ⇒ bascule sur `rescue.lev.1`
- [ ] Les deux fichiers corrompus ⇒ message clair, démarrage normal, aucun crash

---

## Phase S4 — Filet global contre les exceptions

🔴 **P0.** Il n'existe aucun `try/catch` de haut niveau : ni dans `main()`
(`source/main.cpp`), ni autour de `Application::run()` (`Application.cpp:110`).
Toute exception qui s'échappe ⇒ `std::terminate` : **arrêt immédiat, sans log,
sans sauvegarde**.

Chemins qui peuvent lancer, tous réels : `create_directories()`
(`Application.cpp:262`), `resize(l)` (`Event.cpp:47`+), `g_statusConvert.at()`
(`Event.cpp:32`), YAML et jsoncpp dans les imports/exports.

- [ ] `try/catch(...)` dans `main()` : journaliser le type et le message, tenter une
      sauvegarde d'urgence, retourner un code d'erreur explicite
- [ ] `try/catch` **par itération** dans `Application::run()` : une exception dans le
      rendu d'une vue ne doit pas emporter la partie en cours
      *(journaliser, incrémenter un compteur, et n'abandonner qu'après N échecs
      consécutifs pour ne pas boucler à l'infini sur la même erreur)*
- [ ] Vérifier que `Log` est encore vivant au moment du `catch` de `main()`
      *(`Log::invalidate()` est appelé en fin de `main`)*

> **Volontairement écarté** : un handler `SIGSEGV`/SEH qui tenterait d'écrire le
> fichier de secours. Allouer et faire des I/O depuis un handler de signal n'est pas
> sûr. La bonne parade est de **garder `rescue.lev` toujours à jour** (phase S5),
> pas de sauver depuis les décombres.

**Validation**
- [ ] Exception injectée dans une vue ⇒ l'application survit, la partie continue
- [ ] Exception injectée en boucle ⇒ arrêt propre après N échecs, avec sauvegarde
- [ ] Le log contient toujours le type et le message de l'exception

---

## Phase S5 — Autosave déclenché par les événements métier

🟠 **P1.** `autoSave()` sort si moins de 10 s se sont écoulées
(`Application.cpp:255`). Un tirage suivi d'un crash dans les 10 s est perdu — et
c'est le moment le plus sensible : l'écart entre « le numéro annoncé aux joueurs »
et « le numéro enregistré » est un litige en puissance.

- [ ] Déclencher aussi la sauvegarde **sur mutation d'état**, en plus du rythme
      périodique : tirage, annulation de tirage, fin de sous-partie, fin de partie,
      démarrage / arrêt de l'événement
- [ ] Conserver un garde-fou de fréquence minimale pour éviter les rafales
      *(le fichier est petit, mais l'écriture reste synchrone dans la boucle de rendu)*
- [ ] Sauvegarde forcée **avant toute sortie**, y compris sortie sur erreur
      *(aujourd'hui `autoSave()` est appelé en fin de frame `Application.cpp:133` :
      si l'erreur survient au milieu du rendu, on sort sans sauvegarde finale)*
- [ ] Mesurer le coût de l'écriture pour vérifier qu'elle ne provoque pas de
      saccade visible sur l'affichage joueurs

**Validation**
- [ ] `kill -9` immédiatement après un tirage ⇒ le tirage est présent dans `rescue.lev`
- [ ] Aucune saccade perceptible sur la vue plein écran pendant une sauvegarde

---

## Phase S6 — Robustesse GPU et session longue

🟠 **P1.** Sur 4 h avec un vidéoprojecteur, une réinitialisation de pilote GPU
n'est pas hypothétique.

- [ ] **`VK_ERROR_DEVICE_LOST` n'est pas traité.** `checkVkResult`
      (`VulkanContext.cpp:480`) journalise puis appelle `reportError()`, ce qui passe
      `m_state` à `Error` et fait sortir la boucle — sans sauvegarde finale (→ S5) et
      sans tentative de récupération.
  - [ ] minimum : sauvegarde forcée avant la sortie, et message expliquant que la
        partie est récupérable au redémarrage
  - [ ] idéal : recréer device + swapchain sur `DEVICE_LOST`
- [ ] **Attente GPU infinie** : `vkWaitForFences(..., UINT64_MAX)`
      (`VulkanContext.cpp:~505`), avec le commentaire *« wait indefinitely instead of
      periodically checking »*. Si le GPU se bloque, l'application **gèle
      définitivement** : pas de crash, pas de log, pas de sortie — le cas le plus
      pénible en salle. → timeout fini (quelques secondes) avec escalade vers
      « sauvegarde + arrêt propre ».
- [ ] Vérifier les retours non testés à l'initialisation
      (`vkEnumerateInstanceExtensionProperties`, `VulkanContext::init`)
- [ ] Vérifier le comportement sur débranchement / rebranchement de l'écran
      secondaire pendant une partie *(le chemin swapchain `OUT_OF_DATE` est déjà
      traité, `VulkanContext.cpp:498` et `:567` — reste à le valider en vrai)*
- [ ] Test d'endurance : session de 4 h+ avec tirages automatiques, surveillance de
      la mémoire (RSS) et du nombre de handles Vulkan
- [ ] Session d'endurance sous `linux-sanitizer-address` et `linux-sanitizer-leak`

**Validation**
- [ ] 4 h sans crash, sans gel, RSS stable
- [ ] Perte de device simulée ⇒ sauvegarde effectuée et message clair

---

## Phase S7 — Format de fichier portable

🟠 **P1.** `oBs.write(reinterpret_cast<const char*>(&m_start), sizeof(m_start))` sur
des `time_point`, et les longueurs écrites en `sizeof(std::string::size_type)`. La
représentation dépend du compilateur et de l'architecture. Vos images CI proposent
`linux/arm64` (`docker_build_platform`) et vous compilez en GCC **et** Clang : un
`.lev` n'est pas garanti interchangeable.

- [ ] Écrire toutes les longueurs en largeur fixe (`uint64_t`)
- [ ] Écrire les dates en `int64_t` d'epoch plutôt qu'en dump brut de `time_point`
- [ ] Écrire les énumérations en largeur fixe explicite
- [ ] Incrémenter `getSaveVersion()` et **conserver la lecture des versions
      antérieures** *(le mécanisme de version existe déjà, il suffit de s'en servir)*
- [ ] Ajouter un nombre magique en tête de fichier pour rejeter tôt un fichier
      qui n'est pas un `.lev`
- [ ] Envisager une somme de contrôle en queue, pour détecter une corruption
      **avant** de tenter la lecture *(utile pour la bascule S1 vers `rescue.lev.1`)*

**Validation**
- [ ] Un `.lev` écrit par la build GCC x64 se relit par la build Clang x64 et ARM64
- [ ] Un fichier d'une version antérieure se relit toujours
- [ ] Une somme de contrôle invalide est détectée sans lire le corps du fichier

---

## Phase S8 — Qualité et cohérence

🟡 **P2.** Rien de bloquant, mais autant le traiter en passant sur les fichiers.

- [ ] `Event.cpp:99` et `:105` : `m_organizerLogo.string()` et `m_logo.string()` sont
      appelés **dans la boucle d'écriture**. `path::string()` retourne par valeur :
      une `std::string` temporaire est construite à chaque itération → O(n²)
      allocations. Sortir l'appel de la boucle.
- [ ] Remplacer les boucles octet par octet de `read`/`write` par des lectures et
      écritures en bloc *(plus rapide et plus simple à borner)*
- [ ] `Application.cpp:183` (`checkActionEnable`) : `stop_game` n'est activé que si le
      statut est `Finished`. **Question métier** : est-ce voulu qu'on ne puisse pas
      interrompre une partie en cours ? Si l'animateur doit tout arrêter en urgence,
      c'est un problème.
- [ ] `Application.cpp:262` : la logique
      `if (!exists(...) && !empty()) … else if (!is_directory(...))` fonctionne, mais
      par accident pour le cas « chemin vide ». À réécrire lisiblement.
- [ ] Couverture : `gcovr.cfg` exclut `gui/utils`, `gui/views`, `gui/actions`,
      `MainWindow`, `Application`. Or **`Application.cpp` et `FileActions.cpp`
      contiennent l'autosave et les sauvegardes** — le code le plus critique du projet
      n'est pas mesuré. Revoir le périmètre après S1–S5.

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
