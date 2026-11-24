"""Lazy (counterexample-guided) one-vertex extension for large (s,t)-graphs.

Find N with: no K_{s-1} inside N, no independent (t-1)-set inside V \\ N.
Clauses are added only for violations found by exact clique search.
Optional seeding: start from the neighbourhood of an existing vertex (phases).

usage: python3 ext_lazy.py in.txt s t out.txt [max_iters] [seed]
"""
import sys, os, ctypes, random, time
from pysat.solvers import Cadical153

here = os.path.dirname(os.path.abspath(__file__))
lib = ctypes.CDLL(os.path.join(here, '..', 'bin', 'libramsey.so'))
lib.graph_find.restype = ctypes.c_int
lib.graph_find.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_int)]


def load(fn):
    bits = [c for c in open(fn).read() if c in '01']
    n = int(round(len(bits) ** 0.5))
    return n, bytes(int(c) for c in bits)


def gfind(n, adj, mask, k, mode):
    w = (ctypes.c_int * k)()
    r = lib.graph_find(n, adj, bytes(mask), k, mode, w)
    return [w[i] for i in range(k)] if r else None


def main():
    fn, s, t, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
    iters = int(sys.argv[5]) if len(sys.argv) > 5 else 100000
    seed = int(sys.argv[6]) if len(sys.argv) > 6 else 0
    n, adj = load(fn)
    rnd = random.Random(seed)
    S = Cadical153()
    S.add_clause([x + 1 for x in range(n)] + [-(x + 1) for x in range(n)])
    # seed phases from a random existing vertex's neighbourhood
    v0 = rnd.randrange(n)
    S.set_phases([(x + 1) if adj[v0 * n + x] else -(x + 1) for x in range(n)])
    t0 = time.time(); nk = ni = 0
    for it in range(iters):
        if not S.solve():
            print(f'UNSAT after {it} iterations ({nk} clique, {ni} indep clauses)', flush=True)
            return
        m = S.get_model()
        pos = set(v for v in m if v > 0)
        N = [1 if (x + 1) in pos else 0 for x in range(n)]
        K = gfind(n, adj, N, s - 1, 1)
        if K:
            S.add_clause([-(x + 1) for x in K]); nk += 1
            continue
        comp = [1 - b for b in N]
        I = gfind(n, adj, comp, t - 1, 0)
        if I:
            S.add_clause([x + 1 for x in I]); ni += 1
            if (nk + ni) % 200 == 0:
                print(f'  it={it} clauses K={nk} I={ni} |N|={sum(N)} {time.time()-t0:.0f}s', flush=True)
            continue
        rows = [[adj[i * n + j] for j in range(n)] + [N[i]] for i in range(n)] + [N + [0]]
        with open(out, 'w') as f:
            for r in rows:
                f.write(''.join(map(str, r)) + '\n')
        print(f'SAVED n={n+1} deg(new)={sum(N)} after {it} iterations ({nk},{ni}) {time.time()-t0:.0f}s', flush=True)
        return
    print('ITER LIMIT', flush=True)


main()
