from map import * ## "Map Algebra Compiler" package

setupDevices("",0x04,"") # 0x04 == GPU

## Monte Carlo estimation of pi

ds = [32768,32768] # ~ 1e9
bs = [1024,1024]

x  = rand(1234,ds,F32,ROW+BLK,bs)
y  = rand(4321,ds,F32,ROW+BLK,bs)
wc = (x**2 + y**2) < 1.0
pi = 4.0 * zsum(wc*1.0) / prod(ds)

print value(pi)
