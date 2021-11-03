# Événement Loto

Logiciel de gestion d’événement de type loto associatif.

## Construction

Ce projet utilise CMake (version 3.15 ou supérieure) pour se configurer.

### Variables de construction

* `BASE_QT_DIR` (requise) chemin vers Qt (voir les dépendances ci-après).
* `EVL_COVERAGE` active ou non le calcul de la couverture de code.
  (nécéssite `gcovr` voir dépendances)

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
  * [ ] Affichage du règlement
    * [ ] Bouton permettant le ré-affichage à n’importe quel moment
  * [ ] Meilleur affichage du statut de la partie en cours dans la fenêtre principale
  * [ ] Permettre Affichage en plein écran sur le second écran
  * [x] Modification des labels cartons et partie si partie démarrée.
* Thème
    * [ ] Ajout d’icones sur les boutons et actions
    * [ ] Choix du thème graphique
        * [ ] Sélection des couleurs
        * [ ] Possibilité de couleur différente en fonction du sous-niveau
* Interaction
    * [ ] Saisi du numéro du carton gagnant.
    * [ ] Statistiques cartons
    * [x] Désactivation boutons modification partie lors de l’affichage
* Compilation
  * [x] Test compilation OpenBSD
  * [ ] Test compilation msvc (et clang-cl et clang natif)
