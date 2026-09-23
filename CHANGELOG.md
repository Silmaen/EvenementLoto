# Journal des versions

Une ligne par changement. Ce qui est prévu est dans [ROADMAP.md](ROADMAP.md).

## 0.5.0 — en développement

Version de transition vers Conan : outillage, CI et stabilité, sans nouveauté
fonctionnelle.

* Tierces parties : DepManager remplacé par Conan 2, piloté depuis CMake.
* Poetry gère l'environnement Python et les outils de build.
* CI TeamCity décrite dans le dépôt en Kotlin DSL, découpée par sous-projet.
* Nouveaux contrôles CI : style du code, clang-tidy et analyseur statique sur le diff,
  paquet prêt à exécuter isolé.
* Sauvegardes atomiques, deux générations conservées.
* Reprise d'une partie interrompue proposée au démarrage.
* Lecture défensive du format binaire : aucun fichier tronqué n'est accepté.
* Filet contre les exceptions : une vue qui échoue n'emporte plus l'application.
* Attentes Vulkan bornées, incident GPU signalé et partie sauvegardée.
* Format de sauvegarde 7 : nombre magique, somme de contrôle, largeurs fixes.
* Les fichiers des versions 3 à 6 se relisent de nouveau.
* Wayland supporté et WSI Vulkan complet ; X11 demandé par défaut, seul serveur
  permettant d'envoyer l'affichage sur un second écran.
* Tout est lié statiquement sauf le chargeur Vulkan.

## 0.4.1 — 18 février 2026

* Code Qt supprimé.
* Page de documentation affichée dans l'application.
* Sauvegarde périodique de la partie en cours dans `rescue.lev`.
* Documentation d'installation, d'utilisation et de développement.
* Messages d'erreur plus précis, stabilité et performances améliorées.

## 0.4.0 — 15 janvier 2026

* Affichage refait en ImGui : fenêtre principale, à propos, aide, configurations,
  affichage aux joueurs.
* Boutons de numéro à police adaptative et meilleur contraste entre tiré et non tiré.
* Parties non entamées réorganisables et éditables.
* Import/export au format YAML, import JSON mis à jour.

## 0.3.1 — 26 novembre 2023

* Choix de l'écran utilisé en plein écran.
* Paquets Linux.

## 0.3 — 28 novembre 2022

* Affichage des lots à chaque changement de ligne, avec leur valeur.
* Phase indiquée dans le titre de la partie, derniers numéros tirés plus lisibles.
* Valeur de lot ajoutée aux données.
* Statistiques de sortie des numéros et de durée des parties.
* Compilation vérifiée sur Ubuntu 22.04 et Archlinux, mise en place de la CI.

## 0.2 — 30 novembre 2021

* Logo de l'organisateur, règlement et consignes sanitaires affichés.
* Plein écran sur le second écran, configurable.
* Thème graphique complet : couleurs, tailles de police, import/export JSON.
* Icônes sur les boutons et les actions.
* Nouveaux types de parties.
* Sauvegardes binaires versionnées, les anciennes versions restant lisibles.
* Images référencées relativement au fichier de sauvegarde et copiées à côté.
* Règlement importable et exportable en Markdown.
