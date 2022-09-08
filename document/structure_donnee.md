# Structure de données

## Événement

Données propres:

* [enum] status
    * Invalid - Les indispensables ne sont pas définis et doivent l'être
    * MissingPartie - Il manque la définition des parties
    * Ready - Prêt à démarrer
    * Running - Partie en cours
    * Finished - Événement terminé
* [string]* Non de l'organisateur
* [path] Logo organisateur
* [string]* Titre de l'événement
* [path] Logo événement
* [string] Lieu de l'événement
* [string]* Règles du jeu
* [timePoint] Start
* [timePoint] End
* [list rounds] Les différents rounds
    * [enum] Type
    * [enum] Status
    * [int] Numéro
    * [timePoint] Start
    * [timePoint] End
    * [list subround] les sous-rounds
        * [enum] Type
        * [string] Price
        * [float] valeur du lot
        * [list int] liste des tirages
        * [string] Nom du gagnant