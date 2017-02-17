from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",DEV_GPU,"")

## Arguments

assert len(sys.argv) > 1
in_file_path = sys.argv[1]

## Computation

dem = read(in_file_path)
N = prod(dem.datasize())

maxv = zmax(dem)
minv = zmin(dem)
rang = maxv - minv

mean = zsum(dem) / N
geom = zprod(dem) ** (1.0/N)
harm = N / zsum(1.0/dem)
quam = sqrt(zsum(dem**2)/N)
cubm = cbrt(zsum(dem**3)/N)

dev = (dem - mean) ** 2
var = zsum(dev) / N
std = sqrt(var)

norm = (dev - minv) / rang
#norm = ((dem - mean - minv)/rang)**2
nstd  = sqrt(zsum(norm) / N)

#print value(nstd)
eval(maxv,minv,rang,mean,geom,harm,quam,cubm,std,nstd)
print "max: ",value(maxv), "min: ", value(minv), "range: ", value(rang), "mean: ", value(mean), "std: ", value(std)
print "nstd: ", value(nstd), "geom: ", value(geom), "harm: ", value(harm), "quam: ", value(quam), "cubm: ", value(cubm)