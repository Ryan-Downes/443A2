import subprocess as sp
import mkcsv

index = []
i = 1
csv_file = "csv.csv"
mkcsv.makeCSV(csv_file, 34)
while i < 2:
    index.append(i)
    for j in range(0,1):
       # sp.call(["create_random_file", str(i),str(30000000), str(i)])
        print("")

    i = (i * 2)
print(index)