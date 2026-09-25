# Feuille de route

Une ligne par élément. Ce qui est livré passe dans [CHANGELOG.md](CHANGELOG.md).

La **0.5.0**, intercalée comme version de transition vers Conan, est livrée : elle ne
portait pas de nouveauté fonctionnelle, son contenu est dans le changelog. La prochaine
version est la **0.5.1**.

## 0.5.1

* Délai de réactivation des boutons de tirage, réglable, pour donner le tempo.
* Onglet présentateur reprenant en réduit ce qui est à l'écran.

## 0.6.0

* Affichage de la liste des lots sur l'écran présentateur pendant l'annonce.
* Rapport de fin d'événement.
* Édition des noms de gagnant après la fin d'une partie.
* Départage des gagnants multiples, par tirage au sort ou saisie.
* Possibilité de passer l'étape de saisie du gagnant.
* Édition des lots sous forme de liste d'articles.
* Choix de la police de caractère.
* Fenêtres flottantes sous Wayland : aide à côté de la fenêtre principale, grille
  détachable, aperçu en mono-écran.
* Icône et nom de l'application sous Wayland, via l'app-id et un fichier `.desktop`.
* Reprise d'un périphérique Vulkan perdu sans redémarrer l'application.
