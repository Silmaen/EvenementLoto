# Configuration TeamCity

`settings.kts` décrit le projet *Evenement Loto* en Kotlin DSL, en format portable.
C'est la source de vérité : les modifications se font ici, pas dans l'interface du
serveur.

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
