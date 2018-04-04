from map import * ## "Map Algebra Compiler" package
import sys

setupDevices("",DEV_GPU,"")

din = read(sys.argv[1])
dout = din.astype(din.datatype())
write(dout,sys.argv[2])