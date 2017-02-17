from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",DEV_GPU,"")

r1 = read(sys.argv[1])
r2 = read(sys.argv[2])
r3 = read(sys.argv[3])
r4 = read(sys.argv[4])

out = 0 + 0.1*r1 + r2*0.2 + 0.3*r3 + r4*0.4 + 0

write(out,sys.argv[5])
