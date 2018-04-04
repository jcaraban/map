from map import * ## "Map Algebra Compiler" package
import sys

## Arguments

assert len(sys.argv) > 6

sx = int(sys.argv[1])
sy = int(sys.argv[2])
bx = int(sys.argv[3])
by = int(sys.argv[4])
gx = int(sys.argv[5])
gy = int(sys.argv[6])

assert sx%bx==0 and sy%bx==0

size  = 's' + str(sx) + 'x' + str(sy)
block = 'b' + str(bx) + 'x' + str(by)
group = 'g' + str(bx) + 'x' + str(by)
file  = size+'_'+block+'_'+group+'.tif'

## Computation

raster = zeros([sx,sy],F32,bs=[bx,by],gs=[gx,gy])
PI = 3.141592

cx = sx / 2.0 # center x
cy = sy / 2.0 # center y

dif_x = cx - index(raster,D1)
dif_y = cy - index(raster,D2)
dif_max = sqrt(cx**2 + cy**2)

phi  = atan2(dif_y,dif_x) + PI
dist = sqrt(dif_x*dif_x + dif_y*dif_y) / dif_max
rot  = (cos(phi*16 + dist*PI*2) + 1) / 2

elev = (cos(dist*PI*16)+1)/2 * dist * rot
elev = elev * 0.9 + dist * 0.1

raster = elev

## Output

write(raster, file)