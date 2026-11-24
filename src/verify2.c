/* Independent verifier (second implementation, no shared code with clique.h).
 * Computes the exact clique number of G and of its complement using
 * Ostergard's algorithm (Cliquer, "A fast algorithm for the maximum clique
 * problem", 2002) on a plain byte adjacency matrix.
 * usage: verify2 file   (file: n*n 0/1 digits, any separators)
 * prints: n, edges, omega(G), alpha(G)=omega(complement), and a witness of each.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int n;
static unsigned char *A;     /* current graph adjacency (n*n) */
static int *c;               /* Ostergard's c[i] */
static int best, found;
static int *cur, *bestset;
static int *ord;             /* vertex ordering */

#define ADJ(u, v) A[(size_t)(u) * n + (v)]

static void clique_rec(int *U, int usz, int size) {
    if (usz == 0) {
        if (size > best) { best = size; found = 1; memcpy(bestset, cur, sizeof(int) * size); }
        return;
    }
    int *nu = malloc(sizeof(int) * usz);
    while (usz > 0) {
        if (size + usz <= best) break;
        /* pick vertex with smallest ordering index (U kept sorted by position) */
        int i = U[0];
        if (size + c[i] <= best) break;
        /* remove i */
        memmove(U, U + 1, sizeof(int) * (usz - 1)); usz--;
        int m = 0;
        for (int j = 0; j < usz; j++) if (ADJ(ord[i], ord[U[j]])) nu[m++] = U[j];
        cur[size] = ord[i];
        clique_rec(nu, m, size + 1);
        if (found) break;
    }
    free(nu);
}

/* returns clique number, witness in out[] */
static int omega(int *out) {
    /* ordering: Ostergard processes vertices v_n, v_{n-1}, ..., v_1 with S_i = {v_i..v_n};
       use degree ordering (ascending degree first => processed last).  */
    int *deg = calloc(n, sizeof(int));
    for (int u = 0; u < n; u++) for (int v = 0; v < n; v++) deg[u] += ADJ(u, v);
    for (int i = 0; i < n; i++) ord[i] = i;
    /* sort descending degree: position 0 = highest degree */
    for (int i = 1; i < n; i++) { int x = ord[i], j = i - 1; while (j >= 0 && deg[ord[j]] < deg[x]) { ord[j + 1] = ord[j]; j--; } ord[j + 1] = x; }
    best = 0;
    int *U = malloc(sizeof(int) * n);
    for (int i = n - 1; i >= 0; i--) {
        /* S_i = positions i..n-1 ; U = neighbours of ord[i] in S_i (positions > i) */
        int m = 0;
        for (int j = i + 1; j < n; j++) if (ADJ(ord[i], ord[j])) U[m++] = j;
        found = 0;
        cur[0] = ord[i];
        clique_rec(U, m, 1);
        c[i] = best;
    }
    memcpy(out, bestset, sizeof(int) * best);
    free(U); free(deg);
    return best;
}

int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("open"); return 1; }
    size_t cap = 1 << 20, nb = 0; unsigned char *bits = malloc(cap); int ch;
    while ((ch = fgetc(f)) != EOF) if (ch == '0' || ch == '1') { if (nb == cap) bits = realloc(bits, cap *= 2); bits[nb++] = ch - '0'; }
    n = 0; while ((size_t)n * n < nb) n++;
    if ((size_t)n * n != nb) { fprintf(stderr, "not square\n"); return 1; }
    long edges = 0;
    for (int i = 0; i < n; i++) {
        if (bits[(size_t)i * n + i]) { fprintf(stderr, "loop\n"); return 1; }
        for (int j = 0; j < n; j++) if (bits[(size_t)i * n + j] != bits[(size_t)j * n + i]) { fprintf(stderr, "asymmetric\n"); return 1; }
        for (int j = i + 1; j < n; j++) edges += bits[(size_t)i * n + j];
    }
    A = bits; c = calloc(n, sizeof(int)); cur = calloc(n + 1, sizeof(int)); bestset = calloc(n + 1, sizeof(int)); ord = calloc(n, sizeof(int));
    int w1[4096], w2[4096];
    int om = omega(w1);
    /* complement */
    unsigned char *B = malloc((size_t)n * n);
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) B[(size_t)i * n + j] = (i != j) && !bits[(size_t)i * n + j];
    A = B;
    int al = omega(w2);
    /* re-check witnesses directly */
    for (int i = 0; i < om; i++) for (int j = i + 1; j < om; j++) if (!bits[(size_t)w1[i] * n + w1[j]]) { fprintf(stderr, "BAD clique witness\n"); return 2; }
    for (int i = 0; i < al; i++) for (int j = i + 1; j < al; j++) if (bits[(size_t)w2[i] * n + w2[j]]) { fprintf(stderr, "BAD indep witness\n"); return 2; }
    printf("n=%d edges=%ld omega=%d alpha=%d\n", n, edges, om, al);
    printf("clique witness:"); for (int i = 0; i < om; i++) printf(" %d", w1[i]); printf("\n");
    printf("indep witness:"); for (int i = 0; i < al; i++) printf(" %d", w2[i]); printf("\n");
    return 0;
}
