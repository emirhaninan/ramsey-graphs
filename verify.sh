#!/bin/bash
# verify.sh graph.txt s t : checks no K_s and no independent t-set with two
# independent exact codes (bitset colouring B&B enumerator, and Ostergard).
set -e
cd "$(dirname "$0")"
mkdir -p bin
gcc -O3 -march=native -o bin/kenum src/kenum.c
gcc -O3 -march=native -o bin/verify2 src/verify2.c
f=$1; s=$2; t=$3
echo "[kenum]   $(./bin/kenum "$f" $s c 1 q)"
echo "[kenum]   $(./bin/kenum "$f" $t i 1 q)"
echo "[verify2] $(./bin/verify2 "$f" | head -1)   (need omega<$s, alpha<$t)"
