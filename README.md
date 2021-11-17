# Événement Loto

Logiciel de gestion d’événement de type loto associatif.

## Construction

Ce projet utilise CMake (version 3.15 ou supérieure) pour se configurer.

### Variables de construction

* `BASE_QT_DIR` (requise) chemin vers Qt (voir les dépendances ci-après).
* `EVL_COVERAGE` active ou non le calcul de la couverture de code.
  (nécessite `gcovr` voir dépendances)

### Dépendance

* Ce projet dépend de la librairie Qt6 le chemin vers l’installation de Qt6 est passé à CMake par la
  variable `BASE_QT_DIR`. Cette variable est d’ailleurs requise.
* Ce projet nécessite l’installation d’une version de doxygen (version 1.9.1 ou supérieure) pour générer la
  documentation de code. Bien que non essentiel pour la génération du logiciel, être capable de générer une
  documentation reste fondamental. Doxygen doit avoir le module `dot` de disponible (package graphviz)
* Ce projet nécessite `gcovr` pour la génération du rapport de couverture de code. (gcovr peut être installé grâce à
  pip, et pui, du coup nécessite python.)

### Compilateur

Le programme a été correctement compilé avec :

* windows
    * gcc 11.2 ou supérieur (mingw)
    * clang 13 (mingw)

## TODOs

* Affichage:
  * [X] Affichage logo organisateur dans les vues intermédiaires
  * [X] Affichage du règlement
    * [X] Bouton permettant le ré-affichage à n’importe quel moment
  * [X] Meilleur affichage du statut de la partie en cours dans la fenêtre principale
  * [X] Permettre Affichage en plein écran sur le second écran
    * [ ] Permettre le choix/configuration du mode plein écran.
  * [X] Affichage Pause round
  * [x] Modification des labels cartons et partie si partie démarrée.
* Thème
  * [X] Ajout d’icônes sur les boutons et actions
    * [X] Ajout des icônes manquantes
  * [X] Choix du thème graphique
    * [X] Sélection des couleurs
    * [X] Possibilité de couleur différente en fonction du sous-niveau
    * [X] réglage de taille des polices
    * [X] sauvegarde du theme dans les settings
    * [X] export du thème en json
    * [X] import du thème en json
* Interaction
  * [X] Ajout d’autres types de parties
  * [x] Désactivation boutons modification partie lors de l’affichage
  * [X] Edition du règlement dans la fenêtre de configuration de l’événement.
* Sauvegarde et restauration
  * [X] import fiches de lot en json
  * [X] Version des sauvegardes binaire
    * [X] Permettre l’ouverture de vielles versions
  * [X] Chemin relatif au lieu de stockage du fichier binaire pour les images
    * [X] Copie des images dans le dossier contenant le fichier de sauvegarde
  * [X] Gestion du règlement
    * [X] Import/Export via un fichier markdown
    * [X] Sauvegarde dans l’événement
* Compilation
  * [x] Test compilation OpenBSD
  * [ ] Test compilation msvc (et clang-cl et clang natif)
