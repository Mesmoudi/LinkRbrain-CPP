# LinkRbrain

**LinkRbrain** est une plateforme d'analyse et de représententation graphique de données cérébrales, provenant notamment d'**analyses bibliographiques** et d'**échantillonnage génétique**.

<!-- TOC START min:1 max:3 link:true asterisk:false update:true -->
- [LinkRbrain](#linkrbrain)
    - [Présentation](#présentation)
    - [Concepts](#concepts)
        - [Point](#point)
        - [Groupe](#groupe)
        - [Datasets](#datasets)
        - [Organe](#organe)
        - [Corrélation](#corrélation)
        - [Exemple](#exemple)
    - [Installation](#installation)
        - [Prérequis](#prérequis)
        - [Compilation](#compilation)
    - [Usage](#usage)
        - [Initialisation](#initialisation)
        - [Utilisation](#utilisation)
    - [Architecture logicielle](#architecture-logicielle)
        - [Langage de programmation](#langage-de-programmation)
        - [Structure du code](#structure-du-code)
        - [Détails d'implémentation](#détails-dimplémentation)
<!-- TOC END -->

## Présentation

La première version de LinkRbrain, développée entre 2012 et 2014, montrait les défauts suivants :

 - **les calculs de corrélation durent parfois plusieurs minutes**, rendant l'utilisation malaisée
 - **l'intégration d'un nouveau dataset peut être très longue** (plusieurs semaines dans certains cas), ce qui **rend difficile l'expérimentation**
 - le code est dispersé entre **de nombreux programmes écrits en différents langages** (calculs utilisant PHP, Python, MariaDB, Bash et C++), avec des interactions parfois complexes à gérer
 - le code source est **peu souple**, **difficilement modifiable** et **pas toujours bien structuré**
 - l'**installation** sur un nouveau serveur est **complexe**, notamment en raison de la répartition du code et lenteur d'intégration des données

Afin de répondre à ces problèmes, il a été choisi de **redévelopper l'ensemble du back-end** (bases de données et calcul), en conservant la majeure partie du front-end (affichage dans le navigateur de l'utilisateur).

Les principaux changements apportés au back-end sont :

 - la réécriture du code source du back-end dans **une seule base de code en C++**, langage qui présente d'**excellentes performances pour les calculs**
 - une meilleure structuration du code, avec une séparation nette entre **modèles, vues et contrôleurs**

La réalisation de ce projet a été structurée en 4 étapes principales :

 1. Réalisation des algorithmes en C++
 2. Mise en place de la partie applicative
 3. Incorporation de nouvelles données
 4. Connection à BiblioSynth

Le présent livrable correspond à la première étape du développement, à savoir la réalisation des **algorithmes en C++**.

Les outils en ligne de commande développés permettent l'**intégration de données** et l'**analyse de coordonnées spatiales** liées à un organe.

Ces opérations sont **très rapides**, et leur temps d'exécution est respectivement de l'ordre de la minute et du dixième de secondes, contre des semaines et plusieurs minutes dans la version précédente.

A partir du livrable suivant (release-2), les mêmes fonctionnalités seront disponibles via une plate-forme en ligne, avec en outre des possibilités de visualisations en 2D et 3D.

## Concepts

### Point

Un point est caractérisé par quatre nombres réels, à savoir :

 - trois **coordonnées** spatiales (x, y, z)
 - le **poids** rattaché à ce lieu (dans le cas d'imagerie médicale, il s'agit de l'intensité mesurée)

### Groupe

Un groupe est un **ensemble de points**. Il est également caractérisé par un titre, et des métadonnées.

### Datasets

Un dataset est également caractérisé par un titre et des métadonnées. Il est constitué d'un **ensemble de groupes**.

### Organe

Il s'agit d'un organe au sens biologique, caractérisé par un titre et des métadonnées. Chaque organe peut être lié à **plusieurs datasets** le concernant.

### Corrélation

L'utilisateur a la possibilité de corréler une liste de points avec les groupes d'un dataset préalablement intégré.

Pour chacun des groupes du dataset, on effectuera un calcul de corrélation spatiale pour déterminer le score de proximité entre ce groupe et les points fournis par l'utilisateur.

Ce score de proximité est une valeur réelle comprise entre 0 (aucune proximité) et 1 (identité).

On calcule le score de proximité `S(A, A')` entre les points des groupes `A` et `A'` de la façon suivante :

```
S(A, A') = s(A, A') / sqrt(s(A) / s(A'))
```

Où `s(A, A')` est le score non pondéré, obtenu ainsi (en notant `P[i]` les points du groupe `A`, et `P'[i]` les points du groupe `A'`) :

```
s(A, A') = sum(sqrt(w[i] * w[j]) * s(P[i], P[j]))
```

`w[i]` et `w[j]` correspondent respectivement au poids attribués aux points `P[i]` et `P[j]`.

La valeur `s(P[i], P[j])` est le score de corrélation entre deux points. Il peut être déterminée avec cette formules :

`s(P[i], P[j]) = (R - d(P[i], P[j])) / R` quand `d(P[i], P[j]) < R`
`s(P[i], P[j]) = 0` sinon

Où `d(P[i], P[j])` est la distance euclidienne entre les points `P[i]` et `P[j]`.

La valeur `R` correspond au rayon. C'est une constante déterminée à l'avance pour tous les calculs effectués. On utilisera typiquement un rayon de 10 millimètres pour les calculs de corrélation.

Le score entre deux points vaut `1` si les points sont confondus, et `0` si la distance euclidienne entre les points est supérieure ou égale `R`.

Plus de détails sur la méthode de calcul peuvent être trouvés dans la publication suivante, parue en 2014 : [LinkRbrain: multi-scale data integrator of the brain
](https://pubmed.ncbi.nlm.nih.gov/25528112/).

### Exemple

L'usage le plus courant de LinkRbrain est l'étude du cerveau humain.

On peut donc commencer par définir l'**organe** nommé *Adult human brain* (cerveau d'adulte humain). Les métadonnées contiennent entre autres des informations sur le système de coordonnées employé.

À l'intérieur de cet organe, on inclut ensuite les **datasets** suivants : *functions* (tâches cognitives), *genes* (données génétiques fournies par Allen Institute).

Dans chacun de ces datasets, on définit une série de **groupes** de points référencés par un titre :

 - le dataset *functions* comprend environ 500 groupes, dont par exemple *action observation*, *imitation* et *narrative comprehension*, avec pour chacun les coordonnées des points d'activation (dont le poids correspond à l'intensité de l'activation mesrée pendant l'expérience) ; les métadonnées du groupe référencent les publications scientifiques dont sont issus les points d'activation du groupe
 - le dataset *genes* est consitué de plus de 30000 groupes : *DRD1*, *CHRM1*, *MDK*, etc. ; à chaque groupe sont rattachés une série de points, pour lesquels le poids correspond à l'intensité de l'expression des ARNm en ce point ; les métadonnées de chaque groupe donnent des informations sur le gène (implication dans des mécanismes biologique, chromosome, etc.)

## Installation

### Prérequis

Ce programme peut être installé sur tout système Linux Debian≥9 ou Ubuntu≥16, du moment que les commandes `git`, `sudo`, `apt-get` et `apt-cache` sont disponibles.

### Compilation

Pour compiler le programme, commencez par récupérer le code source du programme, en le téléchargeant depuis le dépôt Git de l'ISC :

```bash
git clone https://gitlab.iscpif.fr/MathieuR/linkrbrain-cpp
```

Allez ensuite dans le dossier où ont été copiées les sources :

```bash
cd linkrbrain-cpp
```

Puis placez-vous dans la branche correspondant à la version présentée dans ce document :

```bash
git checkout release-2
git pull origin release-2
```

Il suffit ensuite de lancer la ligne de commande suivante :

```bash
./install
```

Ce script comporte trois étapes principales :

 - installation du compilateur
 - installation des dépendances
 - initialisation des données par défaut

Après exécution de ce script, un lien symbolique `linkrbrain` vers le fichier binaire exécutable pourra être trouvé à la racine du répertoire, et les datasets seront initialisés avec les données par défaut si l'utilisateur en a fait le choix.

## Usage

Ce projet fournit le programmes en ligne de commande `linkrbrain`, qui permet d'effectuer les actions suivantes :

 - `linkrbrain organ add` : définit un nouvel organe sur lequel on va travailler
 - `linkrbrain organ list` : donne la liste des organes déjà définis dans un tableau
 - `linkrbrain organ remove` : supprime un organe donné
 - `linkrbrain dataset add` : définit un nouveau dataset à partir de données existantes
 - `linkrbrain dataset list` : affiche la liste des datasets
 - `linkrbrain dataset correlate` : permet de corréler une liste de points fournis par l'utilisateur avec les groupes de l'un des datasets déjà définis
 - `linkrbrain dataset remove` : supprime un dataset existant

### Utilisation

On peut vérifier que les données ont bien été intégrées en tapant la commande suivante :

```bash
./linkrbrain dataset list
```

Si tout s'est bien déroulé, le tableau suivant s'affiche alors :

```
2 dataset(s) have been registered in 'var/data':
┌────────┬─────────────────┬──────────┬─────────────┬─────────────────┬──────┐
│Organ id│Organ label      │Dataset id│Dataset label│Correlator status│Groups│
├────────┼─────────────────┼──────────┼─────────────┼─────────────────┼──────┤
│1       │Adult human brain│1         │functions        │Cached           │511   │
│1       │Adult human brain│2         │genes        │Cached           │20789 │
└────────┴─────────────────┴──────────┴─────────────┴─────────────────┴──────┘
```

Si l'on souhaite corréler les points de coordonnées (0, 0, 0) et (10, 20, 30) avec les groupes du dataset *functions* en se limitant aux 16 premiers résultats, il suffit de taper la commande suivante:

```bash
./linkrbrain dataset correlate --organ "Adult human brain" --dataset functions --limit 16 --source-type points --source-point 0,0,0 --source-point 10,20,30
```

ou encore :

```bash
./linkrbrain dataset correlate --organ 1 --dataset 1 --limit 16 --source-type points --source-point 0,0,0 --source-point 10,20,30
```

ou :

```bash
./linkrbrain dataset correlate -o1 -d1 -l16 -tpoints -P0,0,0 -P10,20,30
```

Ce qui, dans tous les cas, retournera le résultat suivant :

```
Found 20 correlation results with dataset 'functions':
┌────┬────────┬──────────────────┐
│Rank│Score   │Group label       │
├────┼────────┼──────────────────┤
│1   │0.092619│sleep             │
│2   │0.081003│pleasantness      │
│3   │0.073324│expected reward   │
│4   │0.064075│focusing attention│
│5   │0.061065│pain processing   │
│6   │0.047897│painful stimuli   │
│7   │0.047331│hand motor        │
│8   │0.042345│somatosensory     │
│9   │0.036543│cognitive control │
│10  │0.030976│fluency           │
│11  │0.027640│emotion perception│
│12  │0.026090│pain intensity    │
│13  │0.025723│mood              │
│14  │0.025225│articulatory      │
│15  │0.021163│heart rate        │
│16  │0.016816│stroop            │
│17  │0.014552│rule              │
│18  │0.014540│preparation       │
│19  │0.013972│disgust           │
│20  │0.012073│finger            │
└────┴────────┴──────────────────┘
```

Pour obtenir la documentation complète d'une des commandes, il suffit d'y ajouter le suffixe ` help`. Par exemple, `./linkrbrain dataset correlate help` affiche ceci :

```
linkrbrain dataset correlate - Correlate points with an existing dataset

Usage: linkrbrain dataset correlate [<options>]
       linkrbrain dataset correlate [<command>] [<options>]

Available options:
-h, --help              Generate this helpful message and exit
-o, --organ             Name or identifier of the organ to which the considered dataset is attached (required argument)
-d, --dataset           Name or identifier of the dataset against which the user input has to be correlated (required argument)
-t, --source-type       Source type for the input to be correlated with the dataset; can take one of the following values: 'points' for given coordinates, 'group' for an existing dataset group, 'text' for a text file, 'nifti' for a NIfTI file (required argument)
-P, --source-point      Input points, where floating-point coordinates are separated with spaces (depends on --source-type taking the value 'points', can be multiple)
-D, --source-dataset    Name or identifier of the dataset from which input data should be taken; defaults to the dataset against which correlation will happen; if unspecified, takes the same value as --dataset (depends on --source-type taking the value 'group')
-G, --source-group      Name of the input group to be correlated (depends on --source-type taking the value 'group', required argument)
-E, --source-exact      Perform exact match when searching input group by label (depends on --source-type taking the value 'group', flag)
-T, --source-text       Path to a text file listing input points (depends on --source-type taking the value 'text', required argument)
-N, --source-nifti      Path to the input NIfTI file (depends on --source-type taking the value 'nifti', required argument)
-l, --limit             Maximum number of results to display (defaults to '20')
-u, --uncached          Force just-in-time calculations, event when cache is present (flag)
-i, --interpolate       Use interpolation when calculations are computed using cache (flag)
-f, --format            Data format for output; can be either 'table', 'csv' or 'text' (defaults to 'table')

Available commands:
help                    Generate this helpful message and exit
```

### Configuration manuelle des datasets

Supposons que l'on parte d'une application où aucun organe ni dataset n'est encore configuré.

Commençons par définir l'organe sur lequel on va travailler :

```bash
./linkrbrain organ add --label "Adult human brain"
```

Ceci donne le résultat :

```
Created organ 'Adult human brain' with identifier 1
```

Ensuite, définissons un dataset en intégrant des données fournies par Allen Institute (en supposant que les fichiers `MicroarrayExpression.csv`, `Probes.csv` et `SampleAnnot.csv` soient situés dans le dossier `data/genes`) :

```bash
./linkrbrain dataset add --organ "Adult human brain" --label genes --type allen --source data/genes
```

Le résultat suivant s'affiche alors :

```
Created dataset with label 'genes' and identifier 1
Integrated 20789 groups into dataset, from 'LOC402778' to 'LOC100132346'
Computed correlator
```

Ajoutons ensuite un nouveau dataset, cette fois basé sur une liste de fichiers de barycentres situés à l'emplacement `data/barycenter_txt` :

```bash
./linkrbrain dataset add --organ "Adult human brain" --label functions --type barycenters --source data/barycenter_txt
```

Ce qui nous donne :

```
Created dataset with label 'functions' and identifier 2
Integrated 511 groups into dataset, from 'action' to 'working memory'
Computed correlator
```

## Architecture logicielle

### Langage de programmation

Les calculs nécessaires au fonctionnement de LinkRbrain sont très exigeants en ressources.

Afin d'offrir les meilleures performances possibles, il a donc été décidé que le projet soit développé en **C++**.

Des tentatives d'optimisation avec CUDA ont été évaluées, mais ne se sont pas montrées probantes en raison des temps de latence au niveau des transferts de données entre le CPU et le GPU.

### Structure du code

Le dossier `src` contient toutes les sources nécessaires au projet.

Les classes employées sont chacune située dans un fichier dont le nom est celui de la classe, avec l'extension `.hpp`. Le chemin d'accès à la classe est le même que l'espace de nom.

Par exemple, le code source de la classe `LinkRbrain::Scoring::Correlator` est situé dans le fichier `src/LinkRbrain/Scoring/Correlator.hpp`.

#### Exécutable

Le code source du fichier exécutable est situé à l'emplacement `src/linkrbrain.cpp`. Il s'agit d'un outil en ligne de commande, qui gère un certain nombre d'options.

Les fonctions associées aux commandes exécutées sont définies dans le dossier `src/LinkRbrain/Commands`, à l'espace de nom `LinkRbrain::Commands`.

#### Modèles

Dans le dossier `src/LinkRbrain/Models`, on trouve les classes correspondant aux modèles décrivant les organes, datasets et groupes (respectivement dans les fichiers `Organ.hpp`, `Dataset.hpp` et `Group.hpp`).

De nature très simple, ces modèles permettent de réaliser des opérations de base sur ces éléments à l'aide de méthodes :

 - manipulation des propriétés via des accesseurs
 - recherche, ajout, modification et suppression de sous-élements (garder en tête la hiérarchie Organ -> Dataset -> Group -> Point)
 - sérialisation vers et désérialisation depuis des fichiers binaires (pour le stockage et le chargement des données)

#### Extraction de données

Les classes servant à l'extraction des données depuis des fichiers texte se situent dans `src/LinkRbrain/Parsing`. Elles dérivent de la classe `LinkRbrain::Parsing::Parser`.

Au nombre de deux, elles permettent d'intégrer à un dataset les données extraites depuis les fichiers sous forme de groupes de points :

 - `LinkRbrain::Parsing::GenomicsV1Parser` prend en entrée les fichiers fournis par Allen Institute, et fait correspondre un groupe à chaque gène dosé par l'équipe de recherche.
 - `LinkRbrain::Parsing::FunctionsV1Parser` permet d'intégrer des fichiers fournis sous forme de liste de points, chaque fichier étant interprété comme un nouveau groupe.

#### Calcul de score

Le dossier `src/LinkRbrain/Scoring` contient les classes permettant le calcul de scores de correlation.

 - `LinkRbrain::Scoring::ScoredGroup` est un résultat de corrélation, associant un score sous forme de nombre flottant à une référence vers un groupe.
 - `LinkRbrain::Scoring::Result` sert d'intermédiaire pour le stockage des `ScoredGroup` lors du calcul de corrélation
 - `LinkRbrain::Scoring::Scorer` est la classe qui effectue la plupart des calculs : calcul du score corrélation entre deux points, deux groupes ; elle permet l'utilisation de différentes méthodes de calcul du score (par recouvrement des sphères, ou calcul affine à partir de la distance euclidienne)
 - Dans le dossier `src/LinkRbrain/Scoring/Caching`, on trouve diverses classes dérivant de `LinkRbrain::Scoring::Caching::ScorerCache`, permettant la mise en cache du précalcul pour un groupe donné en utilisant différentes méthodes de stockage des données précalculées.  Cette mise en cache consiste à précalculer le score de corrélation entre chaque point de l'organe et l'ensemble des groupes d'un dataset, afin de rendre les calculs ultérieurs beaucoup plus rapides. Notons que la résolution employée dans le parcours de l'espace de l'organe correspond au paramètre `--resolution` utilisé pour la commande `initialize_dataset`.
 - `LinkRbrain::Scoring::Correlator` est une classe de haut niveau, associant un `LinkRbrain::Model::Dataset`, un `LinkRbrain::Scoring::Scorer`, ainsi qu'un `LinkRbrain::Scoring::Caching::ScorerCache`. On peut ainsi les initialiser ensemble, et sauvegarder ou charger la configuration ainsi que le cache calculé


#### Contrôleurs

Le code source des contrôleurs se trouve dans `src/LinkRbrain/Controllers`. Ceux-ci permettent d'effectuer des opérations de haut niveau sur les organes, et plus particulièrement sur les datasets.

On peut citer les contrôleurs suivants :

 - `LinkRbrain::Controllers::DatasetController` permet de sauvegarder et charger de façon transparente un `LinkRbrain::Model::Dataset`, ainsi que le `LinkRbrain::Scoring::Correlator` qui lui est associé. Cette classe permet en particulier d'effectuer des calculs de corrélation, ainsi que d'accéder directement à l'objet `LinkRbrain::Model::Dataset` qui lui est associé.
 - `LinkRbrain::Controllers::OrganController` sauvegarde et charge les données relatives à un organe, ainsi qu'à tous les `LinkRbrain::Model::Dataset` qui lui sont associés.
 - `LinkRbrain::Controllers::DataController` sauvegarde et charge les données relatives à tous les organes, ainsi qu'à tous les `LinkRbrain::Model::Dataset` qui leur sont associés en prenant comme seul paramètre le chemin d'accès au dossier où sont stockées ces données.

### Détails d'implémentation

Pour plus de détails sur la structuration du code et les différentes classes ainsi que les méthodes qui leur sont associées, vous pouvez vous référer à la documentation complète du code.

Pour ce faire, installer Doxygen avec un simple `sudo apt-get install doxygen`, puis démarrez la commande `doxygen` à la racine du projet. Vous pourrez ensuite ouvrir le fichier `doc/html/index.html` dans votre navigateur web préféré.
