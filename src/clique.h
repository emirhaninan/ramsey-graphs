/* Bitset branch-and-bound clique decision (MCQ-style greedy colouring bound).
 * has_clique(adj, m, k): does the graph on vertices 0..m-1 with adjacency
 * bitsets adj[i] contain a clique of size >= k ?  m <= MAXV.
 */
#ifndef CLIQUE_H
#define CLIQUE_H
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAXV 1024
#define MW (MAXV / 64)

typedef struct { uint64_t w[MW]; } bset;

static int CQ_nw;           /* words in use */
static bset *CQ_adj;        /* adjacency of reordered graph */
static int CQ_k;
static long long CQ_nodes;
static int CQ_found;
static int CQ_best[MAXV], CQ_cur[MAXV];

static inline int bs_count(const bset *a, int nw) {
    int c = 0; for (int i = 0; i < nw; i++) c += __builtin_popcountll(a->w[i]); return c;
}
static inline int bs_empty(const bset *a, int nw) {
    for (int i = 0; i < nw; i++) if (a->w[i]) return 0; return 1;
}
static inline void bs_set(bset *a, int i) { a->w[i >> 6] |= 1ULL << (i & 63); }
static inline int bs_get(const bset *a, int i) { return (a->w[i >> 6] >> (i & 63)) & 1; }

static void cq_expand(bset *P, int depth) {
    CQ_nodes++;
    int nw = CQ_nw;
    /* greedy colouring of P: order[] and col[] */
    int order[MAXV], col[MAXV], cnt = 0;
    bset U = *P;
    int color = 0;
    int total = bs_count(P, nw);
    if (depth + total < CQ_k) return;
    while (!bs_empty(&U, nw)) {
        color++;
        bset Q = U;
        for (int wi = 0; wi < nw; wi++) {
            while (Q.w[wi]) {
                int b = __builtin_ctzll(Q.w[wi]);
                int v = wi * 64 + b;
                Q.w[wi] &= ~(1ULL << b);
                U.w[wi] &= ~(1ULL << b);
                for (int j = 0; j < nw; j++) Q.w[j] &= ~CQ_adj[v].w[j];
                order[cnt] = v; col[cnt] = color; cnt++;
            }
        }
    }
    for (int i = cnt - 1; i >= 0; i--) {
        if (depth + col[i] < CQ_k) return;
        int v = order[i];
        CQ_cur[depth] = v;
        if (depth + 1 >= CQ_k) { CQ_found = 1; memcpy(CQ_best, CQ_cur, sizeof(int) * (depth + 1)); return; }
        bset NP;
        for (int j = 0; j < nw; j++) NP.w[j] = P->w[j] & CQ_adj[v].w[j];
        cq_expand(&NP, depth + 1);
        if (CQ_found) return;
        P->w[v >> 6] &= ~(1ULL << (v & 63));
    }
}

/* adj: m bitsets (indices in 0..m-1). Returns 1 if clique of size >= k exists.
 * If found and witness != NULL, writes k original vertex indices. */
static int has_clique(const bset *adj, int m, int k, int *witness, long long *nodes) {
    if (k <= 0) return 1;
    if (m < k) return 0;
    /* reorder by degree descending (initial order for colouring) */
    int *ord = malloc(sizeof(int) * m), *inv = malloc(sizeof(int) * m), *deg = malloc(sizeof(int) * m);
    int nw = (m + 63) / 64;
    for (int i = 0; i < m; i++) { deg[i] = bs_count(&adj[i], nw); ord[i] = i; }
    /* simple insertion sort by degree desc */
    for (int i = 1; i < m; i++) { int x = ord[i], j = i - 1; while (j >= 0 && deg[ord[j]] < deg[x]) { ord[j + 1] = ord[j]; j--; } ord[j + 1] = x; }
    for (int i = 0; i < m; i++) inv[ord[i]] = i;
    bset *A = calloc(m, sizeof(bset));
    for (int i = 0; i < m; i++) {
        int oi = ord[i];
        for (int wi = 0; wi < nw; wi++) {
            uint64_t x = adj[oi].w[wi];
            while (x) { int b = __builtin_ctzll(x); x &= x - 1; bs_set(&A[i], inv[wi * 64 + b]); }
        }
    }
    CQ_nw = nw; CQ_adj = A; CQ_k = k; CQ_nodes = 0; CQ_found = 0;
    bset P; memset(&P, 0, sizeof P);
    for (int i = 0; i < m; i++) bs_set(&P, i);
    cq_expand(&P, 0);
    if (CQ_found && witness) for (int i = 0; i < k; i++) witness[i] = ord[CQ_best[i]];
    if (nodes) *nodes += CQ_nodes;
    int r = CQ_found;
    free(A); free(ord); free(inv); free(deg);
    return r;
}
#endif
