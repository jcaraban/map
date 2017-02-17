from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

img = read(sys.argv[1])
out = bifilt(img,0.5,0.5)

write(out,sys.argv[2])
