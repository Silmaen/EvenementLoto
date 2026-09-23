# Configuration TeamCity

Le projet *Evenement Loto* est décrit en Kotlin DSL, en format portable. C'est la
source de vérité : les modifications se font ici, pas dans l'interface du serveur.

## Découpage

`settings.kts` ne contient que le `project { }` : ses paramètres, le VCS root, les
templates et l'ordre d'affichage des sous-projets. Le reste suit la hiérarchie :

| Fichier | Contenu |
|---|---|
| `common/Vcs.kt` | le VCS root git |
| `common/Templates.kt` | *Global Build* (onze étapes) et *Tool Build* (deux) |
| `common/Helpers.kt` | le trigger VCS, la fonctionnalité du pont GitHub, les dépendances |
| `common/Factories.kt` | une fonction par famille de configuration |
| `quality/CodeStyle.kt` | la porte que toutes les autres configurations attendent |
| `quality/Analysis.kt` | clang-tidy et l'analyseur statique, les deux portées |
| `quality/Sanitizers.kt` | les quatre sanitizers |
| `build/Linux.kt`, `build/Windows.kt` | les deux projets de build |

`quality/` regroupe trois configurations qui vont ensemble, ce n'est plus un étage de
projet : il n'apportait qu'un paramètre `platform` que ses deux sous-projets déclaraient
déjà, et `Code Style` se lit mieux à la racine puisqu'elle garde tout le reste.
| `packaging/Package.kt` | les deux archives prêtes à exécuter |

Deux contraintes à connaître avant de déplacer quoi que ce soit :

- **Chaque répertoire doit être listé dans `pom.xml`** sous `sourceDirs`, sinon le
  compilateur ne voit pas ses fichiers.
- **Une déclaration dont un autre fichier a besoin ne peut pas vivre dans le `.kts`** :
  les valeurs de haut niveau d'un script Kotlin sont des membres de la classe du script,
  invisibles depuis un `.kt` voisin.

L'ordre d'affichage n'est pas celui des appels `subProject()` / `buildType()` : TeamCity
garde un ordre propre, qu'il faut déclarer avec `subProjectsOrder` et `buildTypesOrder`.
Le premier accepte des `RelativeId`, le second exige les instances de `BuildType`.

Cet ordre **est** la chaîne de dépendances, lue de haut en bas :

```
Code Style  →  Build Linux x64 · Build Windows x64  →  Sanitizers  →  Analysis     Package
                    (Clang dans la chaîne, GCC à côté)
```

`Code Style` ouvre la chaîne mais c'est une configuration, pas un sous-projet : TeamCity
l'affiche donc dans son propre bloc et non dans cette liste. `Package` est en fin de
liste mais hors chaîne : il ne dépend que du style et du build Clang de sa plateforme, et
ne tourne que sur `main`.

## Ce qui n'est pas ici

Le **projet racine** reste géré depuis l'interface. Il porte les paramètres partagés
(`docker_image`, `docker_registry`, `deploy_*`…), la connexion GitHub App et la clé SSH
téléversée. **Aucun secret n'apparaît dans ce dépôt**, et le VCS root ne référence la
clé que par son nom.

Le comportement de chaque preset — tests, couverture, documentation, déploiement, image
Docker — est décrit une seule fois dans `ci/PresetsParameters.json` et poussé en
paramètres TeamCity à l'exécution par les étapes `DefineDockerImage` et
`DefineVariables`. Ne pas le dupliquer ici.

## Vérifier localement

La compilation du DSL produit le XML que le serveur appliquera, ce qui permet de
vérifier une modification avant de la pousser :

```bash
docker run --rm -it --user "$(id -u):$(id -g)" --network host \
  -v "$PWD/.teamcity:/work" -v "$HOME/.m2:/var/maven/.m2" \
  -e MAVEN_CONFIG=/var/maven/.m2 -e HOME=/var/maven -w /work \
  maven:3.9-eclipse-temurin-21 mvn -B -Duser.home=/var/maven teamcity-configs:generate
```

Le résultat est dans `.teamcity/target/generated-configs/` (ignoré par Git). Comparer ce
dossier à un export XML du serveur est le moyen le plus sûr de valider un changement.

## Attention aux identifiants

Le DSL portable identifie une configuration par son `id`. Les `id` sont posés
explicitement (`RelativeId("LotoBranch_Build_LinuxX64_Gcc")`, …) et correspondent à ceux
déjà présents sur le serveur : **les changer ferait perdre l'historique de build** de la
configuration concernée.
