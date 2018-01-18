import sys
import random
import os

FILENAME = "input/veryLongInputFile.csv"
if os.path.isfile(FILENAME):
    os.remove(FILENAME)


products = []

for i in range(1, 21):
    prod = "p_" + str(i)
    products.append(prod)

file = open(FILENAME, 'w+')

for i in range(1000):
    prod1, prod2 = random.sample(products, 2)    
    file.write("{},{}\n".format(prod1, prod2))

file.close()
