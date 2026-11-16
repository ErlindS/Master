import sys
data = open(sys.argv[1], 'rb').read()
nl = data.index(b'\n'); hlen = int(data[:nl])
pre = data[nl+1:nl+1+hlen].decode()
n = int([l for l in pre.splitlines() if l.startswith('p')][0].split()[2])
pos = nl+1+hlen; edges = []
for i in range(n):
    nb = (i + 8) // 8
    row = data[pos:pos+nb]; pos += nb
    for j in range(i+1):
        if row[j//8] & (1 << (7 - j % 8)):
            edges.append((i+1, j+1))
assert pos == len(data), (pos, len(data))
with open(sys.argv[2], 'w') as f:
    for l in pre.splitlines():
        if l.startswith('c'): f.write(l + '\n')
    f.write(f'p edge {n} {len(edges)}\n')
    for u, v in edges: f.write(f'e {u} {v}\n')
print(n, len(edges), sum(u==v for u,v in edges))
