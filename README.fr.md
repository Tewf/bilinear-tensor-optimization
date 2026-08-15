# Décomposition de tenseurs bilinéaires

[![CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml/badge.svg)](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
[![Site](https://img.shields.io/badge/pages-tewf.github.io%2Fbilinear--tensor--optimization-1f6feb)](https://tewf.github.io/bilinear-tensor-optimization/)
[![Licence](https://img.shields.io/badge/licence-MIT-lightgrey)](LICENSE)

> [Read in English](README.md)

Stage de recherche au **LJK** (Laboratoire Jean Kuntzmann, Université Grenoble
Alpes), encadré par **Jean-Guillaume Dumas**, du 21 mai au 15 juillet 2024 — et
ce qu'il en est sorti une fois repris et terminé.

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

**[Rang des applications bilinéaires](rank/)** — la multiplication de polynômes
3×6 sur F3 demande désormais **10 multiplications au lieu des 11 publiées** :
l'étape finale du stage sur cette application n'avait jamais terminé, et le 11
provenait d'une exécution abandonnée. La recherche aboutit maintenant en **9,9
secondes**. Les trois cas qui avaient abouti sont reproduits à l'identique, à
**5,7×, 582× et 283×** la vitesse.

| Application | Naïf | Résultat | Stage |
|---|---|---|---|
| F2 5×5 | 25 | **14** · 2,52 s | 14 · 14,42 s |
| F2 3×8 | 24 | **15** · 5,95 s | 15 · 3460,54 s |
| F2 4×7 | 28 | **16** · 17,80 s | 16 · 5044,06 s |
| F3 3×6 | 18 | **10** · 9,92 s | 11, *n'a pas terminé* |

**[Rendre les opérateurs creux](sparsify/)** — le volet qui ne rapportait aucun
résultat mesuré en a un. Les opérateurs d'encodage de Strassen passent de **12
coefficients non nuls à 10**, et l'opérateur en base alternative sur lequel le
code d'origine était testé passe de **21 à 10**, en quelques millisecondes.
Moins de coefficients non nuls signifie moins d'additions, c'est-à-dire le coût
que le compte de multiplications ne capture pas.

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

## Par où lire

Commencer par les PDF de [`original/`](original/) : ils portent les
dérivations, l'exemple traité de multiplication de polynômes, et les contraintes
que chaque méthode demande. [`original/README.md`](original/README.md) recense
ce qui ne va pas dans le code qu'ils décrivent — la liste à partir de laquelle
la réimplémentation a été construite.

Ensuite [`rank/`](rank/) et [`sparsify/`](sparsify/), chacun avec son README et
un `results.json` dont le site tire ses graphiques.

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
peut ni déborder ni arrondir. **Rien ici n'est jamais un flottant** — les deux
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
