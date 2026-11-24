/* ctypes helpers for circulant graphs Cay(Z_n, S), S symmetric (S[d]=S[n-d]). */
#include "clique.h"
#include <stdio.h>
static long long LR_nodes = 0;
long long lr_nodes(void) { return LR_nodes; }
/* clique of size k containing vertex 0 in Cay(Z_n,S) if want_edges=1,
   or independent set of size k containing 0 if want_edges=0. witness (k ints) */
int circ_find(int n, const unsigned char *S, int k, int want_edges, int *wit) {
    static int vert[MAXV]; int nv = 0;
    static bset adj[MAXV];
    for (int v = 1; v < n; v++) if ((S[v] != 0) == want_edges) vert[nv++] = v;
    if (k <= 1) { wit[0] = 0; return 1; }
    if (nv < k - 1) return 0;
    for (int i = 0; i < nv; i++) {
        memset(&adj[i], 0, sizeof(bset));
        for (int j = 0; j < nv; j++) if (i != j && (S[(vert[i] - vert[j] + n) % n] != 0) == want_edges) bs_set(&adj[i], j);
    }
    int w[MAXV];
    int r = has_clique(adj, nv, k - 1, w, &LR_nodes);
    if (r) { wit[0] = 0; for (int i = 0; i < k - 1; i++) wit[1 + i] = vert[w[i]]; }
    return r;
}

/* General graph: adj is n*n bytes. Find a clique (mode=1) of size k inside
   the vertex subset mask[v]=1, or an independent set (mode=0) of size k
   inside mask[v]=1. */
int graph_find(int n, const unsigned char *adj, const unsigned char *mask, int k, int mode, int *wit) {
    static int vert[MAXV]; int nv = 0;
    static bset A[MAXV];
    for (int v = 0; v < n; v++) if (mask[v]) vert[nv++] = v;
    if (nv < k) return 0;
    for (int i = 0; i < nv; i++) {
        memset(&A[i], 0, sizeof(bset));
        for (int j = 0; j < nv; j++) if (i != j && ((adj[(size_t)vert[i] * n + vert[j]] != 0) == mode)) bs_set(&A[i], j);
    }
    int w[MAXV];
    int r = has_clique(A, nv, k, w, &LR_nodes);
    if (r) for (int i = 0; i < k; i++) wit[i] = vert[w[i]];
    return r;
}
