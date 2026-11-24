"""Greedy chain of one-vertex extensions, preferring HIGH degree for the new
vertex (fewer new independent (t-1)-sets => easier next step).

For each step: solve hitting-set SAT; then repeatedly demand degree >= d+step
with a time limit, keeping the best solution found.

usage: python3 extend2.py in.txt s t outprefix [rounds] [time_per_solve]
"""
import sys, os, threading
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pysat.solvers import Cadical153
from pysat.card import CardEnc, EncType
from pysat.formula import IDPool

src = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'extend.py')).read().split('def main')[0]
exec(src)


def solve_tl(clauses, n, lo, tl):
    pool = IDPool(start_from=n + 1)
    S = Cadical153()
    for c in clauses:
        S.add_clause(c)
    if lo:
        for c in CardEnc.atleast(list(range(1, n + 1)), bound=lo, vpool=pool, encoding=EncType.seqcounter).clauses:
            S.add_clause(c)
    S.conf_budget(int(tl * 2000))
    r = S.solve_limited()
    m = S.get_model() if r else None
    S.delete()
    if r is None:
        return 'TIMEOUT', None
    if not r:
        return 'UNSAT', None
    return 'SAT', [v - 1 for v in m if 0 < v <= n]


def main():
    fn, s, t, outp = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
    rounds = int(sys.argv[5]) if len(sys.argv) > 5 else 10
    tl = float(sys.argv[6]) if len(sys.argv) > 6 else 120
    M = load(fn)
    tmp = outp + '.tmp'
    for r in range(rounds):
        n = len(M)
        save(M, tmp)
        cl = enum(tmp, s - 1, 'c')
        ind = enum(tmp, t - 1, 'i')
        print(f'n={n}: {len(cl)} K{s-1}, {len(ind)} indep {t-1}-sets', flush=True)
        clauses = [[-(v + 1) for v in c] for c in cl] + [[v + 1 for v in I] for I in ind]
        st, N = solve_tl(clauses, n, 0, tl * 10)
        if st != 'SAT':
            print(f'NO EXTENSION ({st}) at n={n}', flush=True)
            return
        best = N
        print(f'  base solution degree {len(N)}', flush=True)
        while True:
            st, N2 = solve_tl(clauses, n, len(best) + 2, tl)
            print(f'  degree >= {len(best)+2}: {st}', flush=True)
            if st != 'SAT':
                break
            best = N2
        M = [row[:] + [0] for row in M] + [[0] * (n + 1)]
        for v in best:
            M[v][n] = M[n][v] = 1
        save(M, f'{outp}_n{n+1}.txt')
        print(f'SAVED n={n+1} degree {len(best)}', flush=True)


main()
