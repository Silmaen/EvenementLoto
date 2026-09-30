# Feuille de route

Une ligne par élément. Ce qui est livré passe dans [CHANGELOG.md](CHANGELOG.md).

La **0.5.1** est livrée, son contenu est dans le changelog. La prochaine version est la
**0.6.0**.

## 0.6.0

* Affichage de la liste des lots sur l'écran présentateur pendant l'annonce.
* Rapport de fin d'événement.
* Édition des noms de gagnant après la fin d'une partie.
* Départage des gagnants multiples, par tirage au sort ou saisie.
* Possibilité de passer l'étape de saisie du gagnant.
* Édition des lots sous forme de liste d'articles.
* Partie improvisée : grille seule, insérable n'importe où dans un événement démarré,
  lots facultatifs saisis au dernier moment, comptée dans le rapport de fin.
* Style des fenêtres revu : palette, typographie, densité et hiérarchie cohérentes.
* Choix de la police de caractère.
* Fenêtres flottantes sous Wayland : aide à côté de la fenêtre principale, grille
  détachable, aperçu en mono-écran.
* Icône et nom de l'application sous Wayland, via l'app-id et un fichier `.desktop`.
* Reprise d'un périphérique Vulkan perdu sans redémarrer l'application.

## 0.7.0

* Catalogue des lots de l'événement : désignation, donateur, valeur, attractivité,
  compatibilité enfant.
* Répartition automatique des lots sur les parties, puis ajustement manuel.
* Valeur croissante des lots au sein d'une partie, de la quine au carton plein.
* Progression de la valeur des lots au long de l'événement, climax à la dernière partie.

## 0.8.0

* Interface traduite, français et anglais, avec sélecteur de langue.
* Documentation d'utilisation en anglais à côté de la version française.
