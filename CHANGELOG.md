# Journal des versions

Une ligne par changement. Ce qui est prévu est dans [ROADMAP.md](ROADMAP.md).

## 0.6.0 — en développement

* Saisie du gagnant à la fin de chaque manche, en remplacement du nom fictif.
* Départage des prétendants à égalité, par choix ou par tirage au sort.
* Étape du gagnant passable, une manche que personne ne réclame n'arrête plus la partie.
* Correction des noms de gagnant après la manche, par le menu « Jeu » > « Gagnants ».
* Lots saisis article par article : désignation, donateur, valeur, attrait, compatibilité enfant.
* Liste détaillée des lots sur l'écran de contrôle, avec valeur et donateur.
* Partie improvisée : ajoutée en cours d'événement, lots facultatifs, jamais devant
  la partie en cours.
* Rapport de fin d'événement : parties, gagnants, lots, donateurs et statistiques,
  affiché, copiable et enregistrable en Markdown.
* Trois habillages au choix, « Ardoise » par défaut : « Nuit » reste l'ancien, « Salle »
  est clair et contrasté, et les couleurs restent retouchables une par une.
* Fenêtre de paramètres réorganisée en onglets — Général, Apparence, Affichage joueurs —
  au lieu de groupes empilés à hauteur fixe qui coupaient leur contenu.
* Choix du serveur d'affichage, X11 ou Wayland, dans les paramètres sous Linux.
* Un panneau d'affichage rogné ne fait plus échouer l'assertion de fin de fenêtre.
* Barre d'outils de taille identique dans tous les habillages : ses boutons suivaient
  l'interligne du thème et rapetissaient sous « Nuit ».
* Le texte désactivé se distingue enfin du texte actif.
* Choix de la police d'interface et de sa taille, appliqués sans redémarrer ;
  un fichier qui n'est pas une police est refusé au lieu de faire tomber l'application.
* Interface dessinée dans la coupe régulière et non plus en gras.
* Identité de bureau : app-id sous Wayland, classe WM sous X11, et un fichier .desktop
  livré avec son icône.
* Périphérique graphique perdu : le rendu, les textures et les glyphes se refont entre
  deux images, sans quitter ni perdre la partie, trois tentatives au plus. Les deux
  backends sont remontés ensemble, sans quoi l'image suivante interrogeait une fenêtre
  disparue.
* Catalogue des lots de l'événement, saisi d'un bloc et indépendant des parties, par le
  menu « Jeu » > « Catalogue des lots ».
* Répartition automatique des lots : valeur croissante de la quine au carton plein, et
  montée réglable au long de l'événement jusqu'à la dernière partie.
* L'attrait d'un lot pèse dans la répartition à côté de son prix, dans la proportion voulue.
* Une partie enfant ne reçoit que les articles marqués compatibles.
* Répartition rejouable : les manches déjà entamées ne sont jamais touchées.
* Un événement enregistré avant le catalogue arrive avec le sien, reconstitué depuis les
  lots déjà répartis à la main ; « Reprendre les lots des parties » le refait à la demande.
* La répartition prévient quand aucun article ne porte de valeur ni d'attrait.
* Onglet « Répartition » : histogramme des valeurs par manche, courbe de progression par
  partie, et le détail article par article de ce qui est mis en jeu où.
* Affectation d'un article changée à la main dans ce tableau, les courbes suivant
  immédiatement ; une manche entamée n'y figure pas et garde ses lots.
* La page « Répartition » s'affiche d'elle-même après une répartition automatique.
* Chaque article du catalogue porte un identifiant : savoir où il est mis en jeu est un
  fait et non une comparaison de désignations.
* Un index de partie hors limites ne fait plus tomber l'application.
* Format de sauvegarde 10 ; les fichiers des versions 3 à 9 se relisent.
* Anciens formats : la valeur d'un lot d'avant la version 8 est portée par son premier
  article, et un catalogue d'avant la version 9 est reconstitué depuis les parties.

## 0.5.1 — 26 septembre 2026

* Nom des archives de livraison suffixé par la plateforme : `linux64`, `win64`.
* Délai de réactivation des commandes de tirage, réglable, pour donner le tempo.
* Onglet « Présentateur » montrant en réduit ce que voient les joueurs.
* Un panneau d'affichage trop étroit ne fait plus échouer l'assertion de mise en page.

## 0.5.0 — 25 septembre 2026

Version de transition vers Conan : outillage, CI et stabilité, sans nouveauté
fonctionnelle.

* Tierces parties : DepManager remplacé par Conan 2, piloté depuis CMake.
* Poetry gère l'environnement Python et les outils de build.
* CI TeamCity décrite dans le dépôt en Kotlin DSL, découpée par sous-projet.
* Nouveaux contrôles CI : style du code, clang-tidy et analyseur statique sur le diff,
  paquet prêt à exécuter isolé.
* Tests d'interface exécutés sans écran, filet à exceptions vérifié par injection.
* Sélecteur de fichiers dessiné dans l'application : plus de dialogue système, plus de
  fenêtre qui passe derrière l'affichage plein écran.
* Dernière dépendance hors ConanCenter retirée : tout vient du dépôt public.
* Un démarrage qui échoue se termine proprement au lieu de planter à l'extinction.
* Réglages par défaut écrits au premier lancement, au lieu d'une erreur dans le journal.
* Ressources et documentation incluses dans le paquet, sans la partie de secours du développeur.
* Documentation d'utilisation revue : sélecteur de fichiers, choix de la carte graphique,
  chemins de menu corrigés.
* Menu d'aide en français, comme le reste de l'application.
* Arborescence du sélecteur : un dossier voisin au nom proche ne se déplie plus à tort.
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
