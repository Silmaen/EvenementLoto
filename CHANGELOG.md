# Journal des versions

Une ligne par changement. Ce qui est prévu est dans [ROADMAP.md](ROADMAP.md).

## 0.6.0 — en développement

* Saisie du gagnant à la fin de chaque manche, en remplacement du nom fictif.
* Départage des prétendants à égalité, par choix ou par tirage au sort.
* Étape du gagnant passable, une manche que personne ne réclame n'arrête plus la partie.
* Correction des noms de gagnant après la manche, par le menu « Jeu » > « Gagnants ».
* Lots saisis article par article : désignation, donateur, valeur, attrait, compatibilité enfant.
* Lots réordonnables par des flèches, dans toutes les listes : c'est l'ordre d'affichage.
* Bouton « Vider la liste », avec confirmation, dans le catalogue et dans les listes de lots des phases.
* Bouton « étape suivante » : libellé juste à chaque état (lots affichés, tirages, fin de partie, pause).
* Fenêtre principale : les panneaux du bas et de droite se réduisent beaucoup plus.
* Pendant l'affichage des lots, la fenêtre principale les détaille en gros caractères à la place des numéros, pour les énoncer.
* Écran de fin de partie : la fenêtre principale affiche « Fin de partie » à la place des numéros ; après une pause, les joueurs voient « Reprise du Jeu ».
* « Tirage aléatoire » grisé (barre d'outils comprise) tant que les lots sont affichés ou que la phase est finie.
* Catalogue : export PDF des lots, une page par partie et par phase, en gros caractères de taille unique pour le jour J, sans les lots non répartis.
* Partie enfant : chaque lot porte un numéro de gagnant ; les lots d'un même gagnant sont regroupés en set (« Gagnant N ») dans le PDF, la fenêtre principale et l'écran des joueurs.
* Import CSV : en partie enfant, la colonne phase est le numéro du gagnant ; la colonne « gagnant » est aussi reconnue.
* Réglages par défaut : habillage Ardoise, police de 24 px.
* Partie enfant : une phase, plusieurs gagnants non nommés ; les ex æquo sont départagés au sort et prennent les lots dans l'ordre.
* Option par partie : afficher ou non les lots aux joueurs (page affichée, mais non remplie).
* Sauvegarde de secours seulement une fois la partie démarrée, plus pendant la définition du programme.
* Numéro de partie masquable à l'affichage joueurs ; le tableur importé désigne la partie par ce numéro, non par son rang.
* Format de sauvegarde 13.
* Import d'un tableur déjà réparti : colonnes « partie » et « phase », lots mis en jeu dans l'ordre des lignes.
* Écran des joueurs et aperçu du présentateur : le donateur est aligné à droite de chaque lot.
* Police et habillage : le changement ne se fait qu'avec « Appliquer » ou « Ok », la taille de police est prise en compte tout de suite.
* Logos copiés à l'enregistrement dans « <nom>_data », à côté de la partie, et retrouvés si elle change de machine.
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
* Numéro de partie attribué d'office à l'ajout, et la partie ajoutée devient la sélection.
* Une pause s'appelle « Pause N » et non « Partie N Pause », avec sa propre numérotation.
* Un lot saisi dans le réglage des parties entre au catalogue au lieu d'y rester invisible.
* Bouton « Ajouter un lot » au-dessus de la liste : sous une liste pleine, il était hors champ.
* Import d'un catalogue depuis un tableur : CSV à point-virgule, virgule ou tabulation,
  colonnes reconnues dans n'importe quel ordre, virgule décimale et symbole euro acceptés.
* Import d'un fichier qui n'est pas un événement : refusé et dit, au lieu d'une exception
  remontée jusqu'à la boucle de rendu, et l'événement en place reste intact.
* Réglages ou fichier de secours pointant sur un répertoire : signalés au lieu d'arrêter
  le démarrage.
* Un lot importé sans désignation ne s'appelle plus « null ».
* Un SVG qui n'en est pas un est refusé au lieu de partir sur la carte graphique.
* Aucune texture n'est tentée sans périphérique graphique.
* Anciens formats : la valeur d'un lot d'avant la version 8 est portée par son premier
  article, et un catalogue d'avant la version 9 est reconstitué depuis les parties.
* Dépôt et CI passés sur GitLab : `.gitlab-ci.yml` remplace la configuration TeamCity.
* Constats de style et d'analyse affichés sur la merge request par le rapport Code Quality.
* Résultats des tests et couverture des lignes visibles dans la merge request.
* Compilation avec clang 23, connu de Conan à partir de la 2.34.
* Jobs Linux de la CI réservés aux runners `amd64` : plus aucun ne part sur le runner arm64.
* Écran d'affichage jamais envoyé sur l'écran de la fenêtre de contrôle tant qu'il en existe un autre.
* Choix de l'écran d'affichage retenu d'un démarrage à l'autre, même si l'ordre des écrans change.
* Plein écran sur tout l'écran et non sur sa zone de travail : plus de liseré en bas.
* Plein écran demandé au gestionnaire de fenêtres : l'affichage n'est plus décalé sous un panneau, bas de l'écran tronqué.
* Écran public aux métriques fixes, indépendantes de l'habillage : son bas de page ne déborde plus.
* Grille publique sans cadre ni espacement entre les numéros, et sans changement de couleur au survol.
* Onglet « Affichage joueurs » des paramètres : les libellés ne s'écrasent plus à gauche.
* Plus de plantage à la mise en pause : démarrer une pause, qui n'a pas de manche, écrivait hors mémoire.
* « Passer » dans la saisie du gagnant termine vraiment la manche, sans gagnant, au lieu de ne rien faire.
* Écran public : logo centré et aussi grand que la place le permet entre le numéro tiré et les horloges.
* Horloges de l'écran public plus grandes par défaut, réglables jusqu'à 5 fois, sans jamais se chevaucher.
* Durée prévue d'une pause réglable, et heure de reprise annoncée aux joueurs pendant la pause.
* Estimation de l'heure de fin, d'après la durée moyenne des manches jouées et les pauses restantes.
* Onglet « Statistiques » refait : tuiles de synthèse, histogramme des sorties par numéro, tableau parties et manches.
* Une pause dont le dossier de diaporama est vide affiche son message au lieu d'un « `<no logo>` » géant.
* Dossier de diaporama vide ou absent lu une seule fois, et non plus à chaque image de la pause.
* Durée de l'événement en cours calculée jusqu'à maintenant, et non jusqu'à une fin encore inconnue.
* Heure de fin prévue de l'événement, et retard ou avance sur le programme dans les statistiques.
* Rythme de tirage : temps moyen entre deux numéros sur l'après-midi et dans la manche en cours.
* Valeur des lots déjà distribués, restés sans gagnant et encore en jeu.
* Format de sauvegarde 11 ; les fichiers des versions 3 à 10 se relisent.
* Nouveau logo, plat et aux couleurs de l'habillage « Ardoise », lisible dès 32 px ; source SVG dans le dépôt.
* Perte du périphérique graphique avec l'affichage sur un second écran : le rendu se refait au lieu de planter.
* Fenêtres de dialogue centrées sur la fenêtre de contrôle et jamais plus grandes qu'elle.
* Tailles exprimées en hauteurs de police : les fenêtres suivent la taille de police choisie.
* Thème complété : cases cochées, liens, curseurs et arrondis ne gardent plus le style ImGui par défaut.
* Configuration de l'événement et des parties réorganisées, sans cadres à hauteur fixe qui coupaient le contenu.
* Liste des lots sur deux lignes quand la place manque, au lieu de champs rognés.
* Barre d'état en français : événement, partie en cours et fichier enregistré.
* Chemin de diaporama saisi au clavier enfin pris en compte.

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
