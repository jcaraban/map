from map import * ## "Map Algebra Compiler" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

img = read(sys.argv[1])

out = stats(img)

write(out,sys.argv[2])
