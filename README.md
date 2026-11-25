# A new lower bound R(4,21) ≥ 254, and an R(8,12) construction

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.24515971.svg)](https://doi.org/10.5281/zenodo.24515971)

This repository contains two Ramsey graphs. A graph on n vertices with no s-clique and no independent set of size t proves R(s,t) ≥ n+1.

| Ramsey number | previous bound | this repository | graph |
|---|---|---|---|
| R(4,21) | ≥ 244 (Google Research, May 2026); ≥ 242 in Radziszowski's survey DS1.18 | **≥ 254** (new lower bound) | 253 vertices |
| R(8,12) | ≥ 511 (Google Research, October 2026) | ≥ 514 | 513 vertices |

The R(8,12) graph is not a record: A. Canli had already obtained R(8,12) ≥ 523 (6 Nov 2026, [quantavane-ramsey-records](https://github.com/arhancanli/quantavane-ramsey-records)), also starting from the Google Research graph. It is kept here as an independent construction by a different method (exact lazy SAT extension).

## Files

- `graphs/r4_21_n253.txt`, `graphs/r8_12_n513.txt`: the graphs, one adjacency-matrix row per line.
- `graphs/R(4,21) >= 254.txt`, `graphs/R(8,12) >= 514.txt`: the same graphs in the format of Google's `improved_bounds` folder.
- `graphs/r4_21_n244.txt`, `graphs/r4_21_n252.txt`: intermediate R(4,21) graphs; `graphs/r8_12_n512.txt`: intermediate R(8,12) graph.
- `src/`: extension code (`extend.py`, `extend2.py`, `swap_ext.py`, `ext_lazy.py` with `libramsey.c`) and two independent exact checkers (`kenum.c`, `verify2.c`).
- `verify.sh`: builds and runs both checkers.

## Provenance

Both graphs extend graphs published by Google Research in `google-research/ramsey_number_bounds/improved_bounds/` (Apache License 2.0, see `NOTICE`): `R(4,21)>=244.txt` was added on 11 May 2026 (commit 351bb65) and `R(8, 12) >= 511.txt` on 22 October 2026 (commit 0d94f8c).

- Restricted to its first 243 vertices, the R(4,21) graph is exactly the graph in `R(4,21)>=244.txt`.
- Restricted to its first 510 vertices, the R(8,12) graph is exactly the graph in `R(8, 12) >= 511.txt`.

The contribution here is the added vertices.

## Method

A new vertex with neighborhood N keeps a graph free of s-cliques and of independent t-sets exactly when:

1. N contains no (s−1)-clique, and
2. N meets every independent (t−1)-set.

Finding N is a SAT problem with one variable per vertex.

**R(4,21), 243 → 253.** The triangles and the independent 20-sets are few enough to list in full, one clause each.
- `extend.py` performs the first step.
- `extend2.py` performs the next eight steps. It adds cardinality constraints so that each new vertex gets as many neighbors as possible, which slows the growth of the number of independent 20-sets.
- `swap_ext.py` performs the last step. The last-added vertex is replaced by two adjacent vertices u, v: N(u) and N(v) are each triangle-free and each meets every independent 20-set, and N(u) ∩ N(v) contains no edge.

Degrees of the added vertices: 35, 49, 47, 48, 48, 47, 44, 47, 42, 43.

**R(8,12), 510 → 513.** The 7-cliques and independent 11-sets are far too many to list. `ext_lazy.py` adds them lazily:
1. The solver proposes N.
2. An exact search looks for a 7-clique inside N or an independent 11-set missed by N.
3. Only that violation is added as a clause, and the solver proposes again.

The loop ends when no violation exists. Degrees of the added vertices: 247, 250, 247.

The search strategy was conceived by the author and the search code developed through many a back and forth with Claude.

## Verification

```
./verify.sh graphs/r4_21_n253.txt 4 21
./verify.sh graphs/r8_12_n513.txt 8 12
```

`verify.sh` runs two independent exact programs:
- `kenum`: bitset branch-and-bound with a coloring bound;
- `verify2`: an implementation of Östergård's algorithm.

A third, separately written exact checker confirms both graphs:
- R(4,21): clique number 3, independence number 20. All three checkers agree.
- R(8,12): no 8-clique, and no independent 12-set. `kenum` agrees.

Google's `verify_bounds.ipynb`, run unchanged, passes on `graphs/R(4,21) >= 254.txt`.

Requirements: gcc. To rerun the searches you also need Python 3 with `python-sat`; for `ext_lazy.py`, also build `bin/libramsey.so` with `gcc -O3 -shared -fPIC -o bin/libramsey.so src/libramsey.c`.

## Citation

Emirhan Inan, *Ramsey graphs: R(4,21) ≥ 254 and R(8,12) ≥ 514* (2026). https://doi.org/10.5281/zenodo.24515971
