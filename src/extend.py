"""One-vertex extension of an (s,t)-Ramsey graph via SAT.

Given G (no K_s, no independent t-set), find N subset V(G) such that
  - N contains no K_{s-1}            (no K_s through the new vertex)
  - N meets every independent (t-1)-set of G (no independent t-set through it)
Then G + v (v adjacent to N) is again an (s,t)-graph on n+1 vertices.

usage: python3 extend.py in.txt s t out.txt [rounds] [target_deg]
"""
import sys, subprocess, os, random
from pysat.solvers import Cadical153
from pysat.card import CardEnc, EncType
from pysat.formula import IDPool

here = os.path.dirname(os.path.abspath(__file__))
KENUM = os.path.join(here, '..', 'bin', 'kenum')


def load(fn):
    bits = [c for c in open(fn).read() if c in '01']
    n = int(round(len(bits) ** 0.5))
    assert n * n == len(bits)
    return [[int(bits[i * n + j]) for j in range(n)] for i in range(n)]


def save(M, fn):
    with open(fn, 'w') as f:
        for r in M:
            f.write(''.join(map(str, r)) + '\n')


def enum(fn, k, mode, limit=5_000_000):
    out = subprocess.run([KENUM, fn, str(k), mode, str(limit)], capture_output=True, text=True).stdout
    sets, tail = [], ''
    for line in out.splitlines():
        if line.startswith('#'):
            tail = line
        elif line.strip():
            sets.append([int(x) for x in line.split()])
    if 'LIMIT' in tail:
        raise RuntimeError('limit reached ' + tail)
    return sets


def extend_once(M, s, t, tmpfn, lo=None, hi=None, seed=0):
    n = len(M)
    save(M, tmpfn)
    cl = enum(tmpfn, s - 1, 'c')
    ind = enum(tmpfn, t - 1, 'i')
    print(f'  n={n}: {len(cl)} K{s-1}s, {len(ind)} independent {t-1}-sets', flush=True)
    pool = IDPool(start_from=n + 1)
    S = Cadical153()
    for c in cl:
        S.add_clause([-(v + 1) for v in c])
    for I in ind:
        S.add_clause([v + 1 for v in I])
    lits = list(range(1, n + 1))
    if lo is not None:
        for c in CardEnc.atleast(lits, bound=lo, vpool=pool, encoding=EncType.seqcounter).clauses:
            S.add_clause(c)
    if hi is not None:
        for c in CardEnc.atmost(lits, bound=hi, vpool=pool, encoding=EncType.seqcounter).clauses:
            S.add_clause(c)
    rnd = random.Random(seed)
    S.set_phases([v if rnd.random() < 0.25 else -v for v in lits])
    if not S.solve():
        return None
    m = S.get_model()
    N = [v - 1 for v in m if 0 < v <= n]
    M2 = [row[:] + [0] for row in M] + [[0] * (n + 1)]
    for v in N:
        M2[v][n] = M2[n][v] = 1
    print(f'  extended: new vertex degree {len(N)}', flush=True)
    return M2


def main():
    fn, s, t, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
    rounds = int(sys.argv[5]) if len(sys.argv) > 5 else 1
    M = load(fn)
    degs = sorted(sum(r) for r in M)
    tmp = out + '.tmp'
    for r in range(rounds):
        M2 = extend_once(M, s, t, tmp, seed=r)
        if M2 is None:
            print(f'NO EXTENSION of n={len(M)}', flush=True)
            break
        M = M2
        out_r = out.replace('.txt', f'_n{len(M)}.txt')
        save(M, out_r)
        save(M, out)
        print(f'SAVED n={len(M)} -> {out}', flush=True)
    if os.path.exists(tmp):
        os.remove(tmp)


main()
