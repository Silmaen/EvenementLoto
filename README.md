# Événement Loto

Logiciel de gestion d’événement de type loto associatif.

## Utilisation

[Utilisation](document/Utilisation.md)

## Construction

Ce projet utilise CMake (version 3.22 ou supérieure) pour se configurer.

### Variables de construction

* `EVL_COVERAGE` active ou non le calcul de la couverture de code.
  (nécessite `gcovr` voir dépendances)

### Dépendance

* Ce projet dépend de la librairie Qt6 (qui doit être installé sur le système)
* Ce projet dépend de python3.10 ou supérieur
* Ce projet dépend de dependency manager [DepManager](https://github.com/Silmaen/DepManager)
    * Pour l'installer par pip: `pip intall depmanager`
    * Les librairies doivent exister en local ou sur un serveur remote configuré avec DepManager.
* Ce projet nécessite `gcovr` pour la génération du rapport de couverture de code.
    * Pour l'installer par pip: `pip intall gcovr`
* Ce projet nécessite l’installation d’une version de doxygen (version 1.9.1 ou supérieure) pour générer la
  documentation de code. Bien que non essentiel pour la génération du logiciel, être capable de générer une
  documentation reste fondamental. Doxygen doit avoir le module `dot` de disponible (package graphviz)
* Depmanager va permettre de gérer les autres dépendances externer
    * fmt
    * spdlog
    * magic_enum
    * googletest
    * Il suffit de se placer dans le répertoire des sources et de lancer `python3 -u ci/DependencyCheck.py`

### Compilateur

Le programme a été correctement compilé avec:

* windows
    * gcc 13 (mingw)
    * clang 17 (mingw)

* linux
    * gcc 12
    * clang 15

## Roadmap

### Version 0.4 (In development)

* Contrôle et paramètres
    * [ ] permettre le choix de la police de caractère
    * [ ] délai de réactivation des boutons de tirage (éviter de tirer des nombres trop vite, donner une idée du tempo)
        * [ ] ajouter le réglage de cette valeur
        * [ ] permettre de régler cette valeur dans les onglets du bas
* Affichage
    * [ ] Affichage sur l'écran présentateur de la liste des lots lot de la phase d'annonce
    * [ ] Ajouter un onglet présentateur pour retour d'écran (affichage identique, réduit de ce qui est à l'écran)
* moteur de jeu
    * [ ] Données
        * [ ] édition de lots sous forme de liste d’articles
    * [ ] Permettre de passer l'étape de saisi du gagnant
    * [ ] Gestion des multiples gagnants
        * [ ] permettre de départager les multiples gagnants (tirage au sort ou saisi)
        * [ ] Résultat
            * [ ] Génération du rapport de fin d'événement
            * [ ] Permettre l'édition des noms de gagnant après la fin de la partie
        * [ ] Flexibilité en jeu
            * [ ] permettre de réorganiser les parties non entamées
            * [ ] permettre l'édition de parties non entamée
* Documentation
    * [ ] Écrire les pages d’aide
* Sauvegarde et restauration
    * [ ] mettre à jour les import/export json

### Version 0.3.1 (Released 26 november 2023)

* Contrôle et paramètres
    * [X] choix de l’écran de plein écran
* Packaging
    * [X] Faire des paquets linux

### Version 0.3 (Released 28 november 2022)

* Affichage
    * [X] affichage des lots
        * [X] à chaque changement de quine
        * [X] affichage de la valeur
    * [X] affichage de la phase dans le titre de la partie
    * [X] améliorer la lisibilité les derniers numéros tirés
    * [X] Police par défaut en gras pour les numéros de la grille
* moteur de jeu
    * [ ] Données
        * [X] ajout de la valeur de lot
        * [X] correction du mouvement de curseur lors de la rédaction des lots
    * [X] ajout de stats
        * [X] de sorties des numéros
        * [X] de temps de parties
    * [ ] Résultat
        * [ ] Génération du rapport de fin d'événement
* Compilation
    * [X] Test compilation Linux
        * [X] Ubuntu 22.04
        * [X] Archlinux
    * [X] Mise en place test CI

### Version 0.2 (Released 30 november 2021)

* Affichage:
    * [X] Affichage logo organisateur dans les vues intermédiaires
    * [X] Affichage du règlement
        * [X] Bouton permettant le ré-affichage à n’importe quel moment
    * [X] Affichage consignes sanitaires
        * [X] Meilleur affichage du statut de la partie en cours dans la fenêtre principale
        * [X] Permettre Affichage en plein écran sur le second écran
            * [X] Permettre le choix/configuration du mode plein écran.
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
    * [X] Version des sauvegardes binaires
        * [X] Permettre l’ouverture de vielles versions
    * [X] Chemin relatif au lieu de stockage du fichier binaire pour les images
        * [X] Copie des images dans le dossier contenant le fichier de sauvegarde
    * [X] Gestion du règlement
        * [X] Import/Export via un fichier markdown
        * [X] Sauvegarde dans l’événement