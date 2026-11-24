"""Delete-one / add-two extension for (s,t)-Ramsey graphs.

If G (n vertices) has no one-vertex extension, try G' = G - w and add two
ADJACENT new vertices u, v with neighbourhoods A, B in V(G'):
  - A and B contain no K_{s-1}                      (no K_s through u or v)
  - A∩B contains no K_{s-2}                          (no K_s through u and v)
  - A and B each meet every independent (t-1)-set of G'
(u~v, so no independent set contains both).  Result: n+1 vertices.

usage: python3 swap_ext.py in.txt s t out.txt [max_tries] [conflict_budget]
"""
import sys, os, random
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pysat.solvers import Cadical153
src = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'extend.py')).read().split('def main')[0]
exec(src)


def try_pair(M, s, t, tmp, budget):
    n = len(M)
    save(M, tmp)
    cl = enum(tmp, s - 1, 'c')
    cl2 = enum(tmp, s - 2, 'c') if s - 2 >= 1 else []
    ind = enum(tmp, t - 1, 'i')
    a = lambda x: x + 1
    b = lambda x: n + x + 1
    S = Cadical153()
    for c in cl:
        S.add_clause([-a(x) for x in c]); S.add_clause([-b(x) for x in c])
    for c in cl2:
        S.add_clause([-a(x) for x in c] + [-b(x) for x in c])
    for I in ind:
        S.add_clause([a(x) for x in I]); S.add_clause([b(x) for x in I])
    S.conf_budget(budget)
    r = S.solve_limited()
    st = 'SAT' if r else ('UNSAT' if r is False else 'TIMEOUT')
    print(f'    n={n}: {len(cl)} K{s-1}, {len(ind)} I{t-1} -> {st}', flush=True)
    if not r:
        return None
    m = set(v for v in S.get_model() if v > 0)
    A = [x for x in range(n) if a(x) in m]; B = [x for x in range(n) if b(x) in m]
    M2 = [row[:] + [0, 0] for row in M] + [[0] * (n + 2) for _ in range(2)]
    for x in A: M2[x][n] = M2[n][x] = 1
    for x in B: M2[x][n + 1] = M2[n + 1][x] = 1
    M2[n][n + 1] = M2[n + 1][n] = 1
    return M2


def main():
    fn, s, t, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
    tries = int(sys.argv[5]) if len(sys.argv) > 5 else 20
    budget = int(sys.argv[6]) if len(sys.argv) > 6 else 2_000_000
    M = load(fn)
    n = len(M)
    # candidates: vertices in the most independent (t-1)-sets first? use lowest degree + random
    deg = [sum(r) for r in M]
    order = sorted(range(n), key=lambda v: deg[v])
    rnd = random.Random(1)
    cand = order[:tries // 2] + rnd.sample(order[tries // 2:], tries - tries // 2)
    tmp = out + '.tmp'
    for w in cand:
        keep = [v for v in range(n) if v != w]
        Mw = [[M[i][j] for j in keep] for i in keep]
        print(f'  delete vertex {w} (deg {deg[w]})', flush=True)
        M2 = try_pair(Mw, s, t, tmp, budget)
        if M2:
            save(M2, out)
            print(f'SAVED n={len(M2)} -> {out}', flush=True)
            return
    print('NO SWAP EXTENSION found', flush=True)


main()
