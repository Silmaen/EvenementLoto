# TODO — Migration technique et stabilité

**Partie I** — migration DepManager → Conan, CI TeamCity en sources, retrait de Copilot.
**Partie II** — durcissement de la stabilité (4 h sans crash, reprise immédiate sur incident).
Les deux parties sont **indépendantes** et peuvent avancer en parallèle.

> Document de travail vivant. Chaque case cochée = fait et validé.
> Les phases sont ordonnées : **ne pas sauter une phase**, chacune isole une cause
> de panne. Les phases 0 à 2 sont indépendantes de Conan.

**État global** : 🟩 migration : phases 0 à 5 + 11 (Linux), 7 quasi complète — stabilité : S0 à S3, S5, S6, S4 partielle
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
| D2 | Intégration | `cmake-conan` **release 0.19.0**, `conan_provider.cmake` **copié** dans `cmake/`. Cette release utilise le générateur **stable `CMakeDeps`** (et non `CMakeConfigDeps` de `develop2`) : pas de fonctionnalité expérimentale, Conan ≥ 2.0.5 suffit. | 2026-09-21 |
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
- [ ] `conan create` OK sous les profils MinGW *(phase 6)*
- [ ] Les dialogues s'ouvrent réellement (ouvrir, enregistrer, sélectionner un dossier)
      *(non vérifiable sans écran : à faire à la main)*
- [ ] Les suppressions de `lsan_suppressions.txt` sont toujours pertinentes *(phase 5)*

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
- [ ] L'application démarre et charge une partie *(non vérifiable sans écran)*

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

- [x] Supprimer `cmake/Depmanager.cmake`
- [x] Supprimer `depmanager.yml`
- [x] `pyproject.toml` : retirer `depmanager = "^0.5.1"`
- [x] `poetry.lock` : régénérer
- [ ] TeamCity : retirer `poetry run dmgr remote add …` du runner « Tool Dependencies »
      (le runner ne garde que `PythonRequirements` + `DefineVariables`)
- [ ] TeamCity : supprimer les paramètres racine `remote_url`, `remote_login`, `remote_passwd`
- [x] **Dépôt `CI/DockerImages`** : `pip install … depmanager gcovr` retiré de
      `_common/builder.sh`
- [x] Images reconstruites et publiées *(fait côté dépôt DockerImages)*
- [x] Mettre à jour `CLAUDE.md` (sections « Dependency management », « External
      Dependencies », « Python Dependencies », « Build System », `conan/`, cibles)
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
| ~~R1~~ | `compiler.cppstd` ⇒ reconstruction depuis les sources | 4 | **confirmé et assumé** : `cppstd` est obligatoire (spdlog, gtest), 7 paquets construits, cache persistant |
| R2 | `IMGUI_API` non exporté en DLL MinGW | 6 | passer imgui en statique (phase 9) |
| ~~R3~~ | ~~clang 22 + `-Weverything`~~ | 1 | **levé** : aucun nouveau diagnostic |
| ~~R4~~ | `xorg/system` échoue faute de `-dev` X11 | 4 | **levé** via `[platform_requires]` + pont `PKG_CONFIG_PATH` |
| R5 | MinGW entièrement `--build=missing` | 6 | cache `~/.conan2` persistant, coût unique |
| ~~R6~~ | `CMakeConfigDeps` expérimental | 4 | **sans objet** : la release 0.19.0 utilise `CMakeDeps`, stable |
| R7 | Venv Poetry partagé entre images gcc et clang du même agent | 2 | même version de Python ; sinon deux venv cohabitent |
| ~~R8~~ | `copy_shared_libraries()` et cibles importées Conan | 4/11 | **résolu autrement** : la fonction ne copiait rien sous Conan ; remplacée par `TARGET_RUNTIME_DLLS` (Windows) — sous Linux le rpath de build et `GET_RUNTIME_DEPENDENCIES` suffisent |

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

- [ ] phase 7 : retirer `depmanager` (et `gcovr`) du `pip install` de
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
- [ ] Vérifier le plein écran de la vue d'affichage sur un second écran en session
      Wayland *(`glfwSetWindowMonitor` est supporté, à valider en vrai)*
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
- [ ] Démarrage réel en session Wayland *(non vérifiable sans écran)*

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
- [ ] Journaliser en en-tête de session le compilateur et la plateforme
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
- [ ] Vérifier que `.tmp` et `rescue.lev.1` ne polluent pas le sélecteur de fichiers
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
- [ ] `FileActions::LoadFileAction` : exploiter l'échec de lecture pour prévenir
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
- [ ] Nettoyer les archives au-delà de N *(elles s'accumulent aujourd'hui)*
- [ ] Après une reprise acceptée, supprimer le fichier de secours à la première
      sauvegarde explicite *(il est conservé pour l'instant, ce qui est le côté sûr)*

**Validation** — `test/lib_test/test_Rescue.cpp`, 6 tests
- [x] Enregistrement, détection puis rechargement d'une partie en cours
- [x] Zone vide : rien n'est proposé
- [x] Deuxième enregistrement : la génération précédente est conservée
- [x] **Génération la plus récente tronquée ⇒ bascule sur la précédente**
- [x] Un événement non repris (`Invalid`) n'est pas proposé
- [x] L'archivage conserve le fichier
- [ ] `kill -9` en pleine partie sur l'application réelle *(non vérifiable sans écran)*

## Phase S4 — Filet global contre les exceptions

🔴 **P0.** Il n'existe aucun `try/catch` de haut niveau : ni dans `main()`
(`source/main.cpp`), ni autour de `Application::run()` (`Application.cpp:110`).
Toute exception qui s'échappe ⇒ `std::terminate` : **arrêt immédiat, sans log,
sans sauvegarde**.

Chemins qui peuvent lancer, tous réels : `create_directories()`
(`Application.cpp:262`), `resize(l)` (`Event.cpp:47`+), `g_statusConvert.at()`
(`Event.cpp:32`), YAML et jsoncpp dans les imports/exports.

- [x] `try/catch` dans `main()` : type et message journalisés, `EXIT_FAILURE` renvoyé.
      Le gestionnaire de dernier recours est `noexcept`, donc rien ne peut s'échapper
      *(confirmé par `bugprone-exception-escape`, qui est désormais vert)*
- [ ] Sauvegarde d'urgence depuis le gestionnaire de `main()`
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

**Faite.** L'écart entre « le numéro annoncé aux joueurs » et « le numéro enregistré »
est désormais nul.

- [x] `Application::saveProgress()` (public, force la sauvegarde) appelé après chaque
      mutation : tirage, annulation de tirage, changement d'état de partie
      (`RandomPickAction`, `CancelPickAction`, `GameNextActions`)
- [x] `autoSave(bool iForce)` : le garde-fou de 10 s reste pour le rythme périodique
- [x] Sauvegarde forcée **avant toute sortie**, boucle principale comme chemin d'erreur
- [x] `reportError()` sauvegarde, et ne le fait qu'à la première transition vers `Error`
- [ ] Mesurer le coût de l'écriture sur la fluidité d'affichage *(fichier petit, mais
      l'écriture reste synchrone dans la boucle)*

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
- [ ] Test d'endurance 4 h+ avec tirages automatiques, suivi du RSS et des handles
- [ ] Session d'endurance sous `linux-sanitizer-address` et `linux-sanitizer-leak`

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
