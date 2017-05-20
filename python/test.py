from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

img = read(sys.argv[1])
out = (img <= 0.5)*img + (img > 0.5)*(1-img)

write(out,sys.argv[2])
