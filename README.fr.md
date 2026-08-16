# Décomposition de tenseurs bilinéaires

[![CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml/badge.svg)](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
[![Site](https://img.shields.io/badge/pages-tewf.github.io%2Fbilinear--tensor--optimization-1f6feb)](https://tewf.github.io/bilinear-tensor-optimization/)
[![Licence](https://img.shields.io/badge/licence-MIT-lightgrey)](LICENSE)

> [Read in English](README.md)

Stage de recherche au **LJK** (Laboratoire Jean Kuntzmann, Université Grenoble
Alpes), encadré par **Jean-Guillaume Dumas**, du 21 mai au 15 juillet 2024,
et ce qu'il en est sorti une fois repris et terminé.

Le rang d'une application bilinéaire est le nombre de multiplications
nécessaires pour la calculer. Le sept-au-lieu-de-huit de Strassen pour les
matrices 2×2 est l'origine de la multiplication matricielle rapide, et trouver
de telles décompositions dans le cas général reste un problème ouvert. Ce dépôt
en est une recherche, dans deux directions.

[`original/`](original/) contient le stage exactement tel qu'il a été rendu et
ne bouge plus. Tout le reste est une réimplémentation corrigée et testée en C++,
mesurée contre lui. Chaque nombre ci-dessous est produit par du code de ce
dépôt, et la
[CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
rejoue le tableau entier à chaque push.

## Ce qu'a donné le fait de le terminer

**[Rang des applications bilinéaires](internship_heuristic/)** : la multiplication de polynômes
3×6 sur F3 demande désormais **10 multiplications au lieu des 11 publiées** :
l'étape finale du stage sur cette application n'avait jamais terminé, et le 11
provenait d'une exécution abandonnée. La recherche aboutit maintenant en **7,49
secondes**. Les trois cas qui avaient abouti sont reproduits à l'identique, à
**10×, 1033× et 520×** la vitesse.

| Application | Naïf | Résultat | Stage |
|---|---|---|---|
| F2 5×5 | 25 | **14** · 1,41 s | 14 · 14,42 s |
| F2 3×8 | 24 | **15** · 3,35 s | 15 · 3460,54 s |
| F2 4×7 | 28 | **16** · 9,69 s | 16 · 5044,06 s |
| F3 3×6 | 18 | **10** · 7,49 s | 11, *n'a pas terminé* |

**[Rendre les opérateurs creux](matrix_sparsification/)** : le volet qui ne rapportait aucun
résultat mesuré en a un. Les opérateurs d'encodage de Strassen passent de **12
coefficients non nuls à 10**, et l'opérateur en base alternative sur lequel le
code d'origine était testé passe de **21 à 10**, en quelques millisecondes.
Moins de coefficients non nuls signifie moins d'additions, c'est-à-dire le coût
que le compte de multiplications ne capture pas.

**La recherche exacte tranche les petites applications** et, pour la première
fois ici, minore une grande. F2 5×5 n'admet aucun algorithme à 9, 10, 11 ni 12
produits, chacun exclu de façon exhaustive : son rang vaut donc **13**, la
recherche prouvant qu'il vaut au moins 13 et Barbulescu, Detrey, Estibals et
Zimmermann ayant trouvé 27 décompositions à 13 en 2012. Le 14 de l'heuristique
n'est donc pas optimal. Là où elle aboutit, elle retrouve le 3 de Karatsuba, le 5 de
l'exemple traité dans la rédaction, et les 3 et 6 classiques pour la
multiplication dans GF(4) et GF(8).

**Et les deux volets forment de nouveau une chaîne.** La recherche de rang
reconstruit les opérateurs ⟨L, R, P⟩ à partir de sa décomposition et les écrit ;
la sparsification est précisément ce à quoi ils servent :

```sh
minimise-rank fixtures/f2_5x5.tensor --emit-operators out   # 25 -> 14 multiplications
sparsify-operator out_L.sms                           # 31 -> 27 coefficients non nuls
```

## Ce que cachait le tableau 1

Transformer le tableau publié en jeux de test a fait apparaître un point que la
rédaction ne dégage pas. Sa troisième étape, coûteuse, n'a amélioré le résultat
que dans **un cas sur quatre**. Sur 4×7 elle a dépensé 5020 de ses 5044 secondes
pour retourner le rang que l'étape précédente avait déjà ; sur F3 3×6 elle n'a
jamais terminé.

Autrement dit, l'étape qui coûte presque tout n'apporte presque rien, et
l'accélérer revient à optimiser la partie qui ne paie pas. C'est pourquoi le
résultat intéressant vient ici d'une exécution menée à son terme plutôt que
d'une exécution accélérée.

## Ce qu'il y a, et où

```
original/                le stage tel qu'il a été rendu, figé
COVERAGE.md              chacune de ses 89 fonctions, et ce qu'elle est devenue
linear_algebra/          l'algèbre linéaire exacte sur GF(p) et sur Q, partagée par tout
formats/                 fichiers tenseur, matrice dense et SMS
cli/                     la seule chose que partagent les commandes : une horloge
testing/                 l'aide aux assertions que tous les tests utilisent
run_limits/              la mémoire et le nombre de cœurs qu'une exécution peut prendre
internship_heuristic/    volet 1 : l'heuristique du stage, corrigée
exhaustive_search/       volet 1 : trancher le nombre minimal de produits
map_construction/        volet 1 : construire les applications que les recherches parcourent
matrix_sparsification/   volet 2 : le moins de coefficients non nuls dans un opérateur
fixtures/                les applications et opérateurs servant aux mesures
tools/                   le vérificateur de couverture qu'exécute la CI
site/                    la feuille de style et les graphiques de la page publiée
```

Quatre outils en ligne de commande : **`minimise-rank`** (heuristique),
**`decide-rank`** (exact), **`make-tensor`** (construire une application),
**`sparsify-operator`**.

| Dossier | Ce que c'est | Commencer par |
|---|---|---|
| **[`original/`](original/)** | Le stage de 2024, déplacé ici par un simple renommage et jamais modifié depuis. Deux PDF avec les dérivations, plus le Julia et le Python correspondants. | [son README](original/README.md) : ce qui a été rendu, et la liste de défauts qui a servi de base à la réécriture |
| **[`formats/`](formats/)** | Lecture et écriture : tenseurs, matrices denses, et SMS, le format que parlent LinBox et Givaro. | [son README](formats/README.md) : les trois formats, et pourquoi un fichier d'opérateur est rationnel |
| **[`fixtures/`](fixtures/)** | Les données d'entrée, écrites en toutes lettres pour que le code soit vérifié contre des octets et non contre un générateur. Les `.tensor` sont des applications bilinéaires, les `.matrix` des opérateurs. | [son README](fixtures/README.md) : le tableau de résultats publié, et ce qu'il dit vraiment |
| **[`linear_algebra/`](linear_algebra/)** | La couche partagée : matrice, rang, sous-espace engendré, résolution exacte, décomposition en rang 1. Paramétrée par le corps, donc une seule implémentation sert les deux volets. | [son README](linear_algebra/README.md) : le coût de chaque opération, et où les rationnels exacts cessent d'être gratuits |
| **[`internship_heuristic/`](internship_heuristic/)** | Volet 1, l'heuristique. Un fichier par étape, nommé d'après ce qu'elle garantit : `smallest_basis` est exacte pour la base qu'elle choisit, `minimise_rank` ne garantit rien. `commands/` produit `minimise-rank`. | [son README](internship_heuristic/README.md) pour les résultats, [`method.md`](internship_heuristic/method.md) pour les algorithmes et leur complexité |
| **[`exhaustive_search/`](exhaustive_search/)** | Volet 1, la décision complète : existe-t-il un algorithme à exactement `k` produits ? Exponentielle, elle tranche les petites applications et minore les grandes. `commands/` produit `decide-rank`. | [`exhaustive_search.h`](exhaustive_search/exhaustive_search.h) pour ce qu'elle décide et ce qu'elle coûte |
| **[`map_construction/`](map_construction/)** | Volet 1, les entrées : construire les applications bilinéaires que chaque méthode parcourt ensuite. `commands/` produit `make-tensor`. | [`map_construction.h`](map_construction/map_construction.h) |
| **[`matrix_sparsification/`](matrix_sparsification/)** | Volet 2. `heuristic_sparsifier` est la construction par base de lignes de Mohamed, `oracle_sparsifier` les deux oracles exacts de l'article. `commands/` produit `sparsify-operator`. | [son README](matrix_sparsification/README.md) pour les résultats, [`method.md`](matrix_sparsification/method.md) pour les algorithmes et leur complexité |
| **[`famous_tensors.md`](famous_tensors.md)** | Les tenseurs dont discute la littérature, passés dans les deux recherches : le ⟨2,2,2⟩ de Strassen tranché exactement, l'état W, la convolution cyclique, et où chaque méthode s'arrête. | lui, pour ce que font les deux méthodes sur des applications pour lesquelles ce dépôt n'a pas été écrit |
| **[`COVERAGE.md`](COVERAGE.md)** | Chacune des 89 fonctions de l'original, et ce qu'elle est devenue : portée, remplacée, supplantée, ou encore à venir. La CI échoue s'il manque une ligne. | lui, pour savoir si quelque chose a survécu |
| **[`site/`](site/)** | `style.css`, `chart.js` et `nav.js` de [la page](https://tewf.github.io/bilinear-tensor-optimization/), partagés avec tewf.github.io. Aucune étape de compilation, aucun CDN. | [`index.html`](index.html) à la racine |

Chaque dossier de méthode contient le code lui-même, ses `tests/` et un
`commands/` qui produit les points d'entrée en ligne de commande. Les documents
du volet 1, son `README.md`, son `method.md` pour les algorithmes et le
`results.json` dont le site tire ses graphiques, sont dans
`internship_heuristic/`.

**Par où commencer, selon ce que l'on cherche.** Pour les mathématiques, les
deux PDF de [`original/`](original/). Pour ce qui n'allait pas et ce qui a
changé, [`original/README.md`](original/README.md). Pour les résultats,
[`internship_heuristic/README.md`](internship_heuristic/README.md) et [`matrix_sparsification/README.md`](matrix_sparsification/README.md).
Pour le code, `linear_algebra/` d'abord ; les deux volets sont minces par-dessus. Pour
les algorithmes énoncés précisément et leur coût en temps et en espace, les deux
fichiers `method.md`.

## Compilation

Demande un compilateur C++20, CMake et **Givaro**
(`sudo apt install libgivaro-dev`).

```sh
cmake -B build -G Ninja && cmake --build build
ctest --test-dir build            # le tableau entier, une trentaine de secondes
ctest --test-dir build -LE slow   # sans les trois recherches coûteuses
```

Givaro vient de l'équipe CASYS du LJK, ce qui en fait la bibliothèque de
l'encadrant pour précisément ce problème. Elle fournit ce qui porte le
raisonnement : l'inverse modulaire et une arithmétique rationnelle exacte qui ne
peut ni déborder ni arrondir. **Rien ici n'est jamais un flottant** : les deux
volets font des recherches sur des rangs et sur des nombres de zéros, donc une
réponse presque juste répond à une autre question.

## Où cela s'arrête

Toutes les méthodes ici sont des heuristiques. Aucune ne prouve que la
décomposition trouvée est optimale, et aucune ne règle le problème du rang
bilinéaire, qui reste ouvert. Le 10 sur F3 3×6 est une meilleure décomposition
que celle enregistrée, pas une affirmation sur le rang réel.

---

Le dépôt de travail d'origine est sur le GitLab de l'UGA (`hamlilm/AltBase`) et
n'est pas accessible publiquement ; ceci en est la copie publique.

## Licence et crédits

Le code et les textes sont sous MIT ; voir [LICENSE](LICENSE). **Le
[NOTICE](NOTICE) compte ici** : la licence MIT ne couvre que mon propre travail.
Les algorithmes proviennent d'articles publiés, cités et non redistribués, et
Givaro appartient à ses auteurs.
