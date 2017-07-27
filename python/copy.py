from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",DEV_GPU,"")

din = read(sys.argv[1])
dout = din.astype(din.datatype())
write(dout,sys.argv[2])