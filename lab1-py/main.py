import numpy as np


def ld(A):
    n = len(A)
    LD = np.array(A, float)
    for j in range(n):
        v = np.zeros(j + 1)
        v[:j] = LD[j, :j] * LD[range(j), range(j)]
        v[j] = LD[j, j] - np.dot(LD[j, :j], v[:j])
        LD[j, j] = v[j]
        LD[j + 1:, j] = (LD[j + 1:, j] - np.dot(LD[j + 1:, :j], v[:j])) / v[j]
    return LD


def ld_solve(A, b):
    LD = ld(A)
    print(LD)
    b = np.array(b, float)
    for i in range(1, len(b)):
        b[i] = b[i] - np.dot(LD[i, :i], b[:i])
    b[:] = b[:] / LD[range(len(b)), range(len(b))]
    for i in range(len(b) - 1, -1, -1):
        b[i] = (b[i] - np.dot(LD[i + 1:, i], b[i + 1:]))
    return b


A = np.array([[1, 3, 4, 5, 6],
              [3, 1, 2, 4, 7],
              [4, 2, 1, 5, 8],
              [5, 4, 5, 1, 9],
              [6, 7, 8, 9, 1]], float)

b = np.array([4, 3, 5, 7, 5], float)

print(A)
print(b)
print()
print(ld_solve(A, b))

