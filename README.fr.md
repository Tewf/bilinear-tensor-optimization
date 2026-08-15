# Décomposition de tenseurs bilinéaires

> [Read in English](README.md)

Stage de recherche au **LJK** (Laboratoire Jean Kuntzmann, Université Grenoble
Alpes), encadré par **Jean-Guillaume Dumas**.

Le rang d'une application bilinéaire est le nombre de multiplications nécessaires
pour la calculer. Le sept-au-lieu-de-huit de Strassen pour les matrices 2×2 est
l'origine de la multiplication matricielle rapide, et trouver de telles
décompositions dans le cas général reste un problème ouvert. Ce dépôt en est une
recherche, dans deux directions.

| Dossier | Ce qu'il contient |
|---|---|
| [`Bilinear_Rank_over_Finite_Field/`](Bilinear_Rank_over_Finite_Field/) | Une heuristique qui prend une application bilinéaire sur un corps fini et en renvoie une autre engendrant le même espace avec moins de composantes de rang 1. Rédaction, plus les implémentations Python et Julia. |
| [`Sparsifying_Matrices/`](Sparsifying_Matrices/) | Rendre creux les opérateurs sur lesquels repose la multiplication rapide : `argmin nnz(AU)` sur les `U` inversibles. Rédaction, implémentation Python, notebook Julia. |

Commencer par le PDF de l'un ou l'autre dossier : ils portent les dérivations,
l'exemple traité de multiplication de polynômes, et les contraintes que chaque
méthode demande.

## Où cela s'arrête

Les deux sont des heuristiques. Aucune ne prouve que la décomposition trouvée est
optimale, et aucune ne règle le problème du rang bilinéaire, qui reste ouvert.

---

Le dépôt de travail d'origine est sur le GitLab de l'UGA (`hamlilm/AltBase`) et
n'est pas accessible publiquement ; ceci en est la copie publique.
