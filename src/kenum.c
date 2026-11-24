/* Enumerate (or count) all k-cliques / independent k-sets of a graph.
 * usage: kenum file k mode [limit]
 *   file: adjacency matrix, n rows of 0/1 (any separators; first line may be n)
 *   mode: c = cliques, i = independent sets
 * prints each set as a line of vertex indices, final line "# count=..."
 * If limit reached, prints "# LIMIT".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "clique.h"

static int n, K;
static bset adj[MAXV];
static long long cnt = 0, limit = -1;
static int cur[MAXV];
static int nw;
static int quiet = 0;

/* exact enumeration of k-cliques with colouring bound */
static int enum_rec(bset *P, int depth) {
    if (depth == K) {
        cnt++;
        if (!quiet) { for (int i = 0; i < K; i++) printf(i ? " %d" : "%d", cur[i]); printf("\n"); }
        return (limit >= 0 && cnt >= limit);
    }
    int need = K - depth;
    if (bs_count(P, nw) < need) return 0;
    /* greedy colouring bound */
    int order[MAXV], col[MAXV], c = 0, color = 0;
    bset U = *P;
    while (!bs_empty(&U, nw)) {
        color++;
        bset Q = U;
        for (int wi = 0; wi < nw; wi++) while (Q.w[wi]) {
            int b = __builtin_ctzll(Q.w[wi]); int v = wi * 64 + b;
            Q.w[wi] &= ~(1ULL << b); U.w[wi] &= ~(1ULL << b);
            for (int j = 0; j < nw; j++) Q.w[j] &= ~adj[v].w[j];
            order[c] = v; col[c] = color; c++;
        }
    }
    for (int i = c - 1; i >= 0; i--) {
        if (col[i] < need) return 0;
        int v = order[i];
        cur[depth] = v;
        bset NP; for (int j = 0; j < nw; j++) NP.w[j] = P->w[j] & adj[v].w[j];
        if (enum_rec(&NP, depth + 1)) return 1;
        P->w[v >> 6] &= ~(1ULL << (v & 63));
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage\n"); return 1; }
    FILE *f = fopen(argv[1], "r"); K = atoi(argv[2]); char mode = argv[3][0];
    if (argc > 4) limit = atoll(argv[4]);
    if (argc > 5) quiet = 1;
    /* read all 0/1 digits */
    static unsigned char bits[MAXV * MAXV]; long long nb = 0; int ch;
    while ((ch = fgetc(f)) != EOF) if (ch == '0' || ch == '1') bits[nb++] = ch - '0';
    n = 0; while ((long long)n * n < nb) n++;
    if ((long long)n * n != nb) { fprintf(stderr, "not square: %lld\n", nb); return 1; }
    nw = (n + 63) / 64;
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) {
        if (bits[i * n + j] != bits[j * n + i]) { fprintf(stderr, "asym %d %d\n", i, j); return 1; }
        int e = bits[i * n + j];
        if (i != j && ((mode == 'c') ? e : !e)) bs_set(&adj[i], j);
    }
    for (int i = 0; i < n; i++) if (bits[i * n + i]) { fprintf(stderr, "loop at %d\n", i); return 1; }
    bset P; memset(&P, 0, sizeof P); for (int i = 0; i < n; i++) bs_set(&P, i);
    int lim = enum_rec(&P, 0);
    printf("# n=%d k=%d mode=%c count=%lld%s\n", n, K, mode, cnt, lim ? " LIMIT" : "");
    return 0;
}
