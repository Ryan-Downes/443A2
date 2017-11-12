import sys
import random
def makeCSV(filename, n):
    letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    with open(filename, 'w') as f:
        for _ in range(n):
            row = []
            for a in range(100):
                row.append(''.join([random.choice(letters) for j in range(10)]))
            print(",".join(row), file=f)

    print("Generated %d random tuples in %s." % (n, filename))
