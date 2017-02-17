from map import * ## "Parallel Map Algebra" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## Arguments

argv = sys.argv
argc = len(argv)
assert argc > 3

file = argv[1]
ds = [int(argv[2]),int(argv[3])]
bs = [256,256]

if argc > 4:
	bs = [int(argv[4]),int(argv[4])]
if argc > 5:
	N = int(argv[5])

assert ds[0] % bs[0] == 0 && ds[1] % bs[1] == 0

## Initialization

one = ones(ds,F32,ROW+BLK,bs)

S = 1e-6 # Horizontal stress
T = 1e-6 # Vertical stress
V = 1e-6 # Velocity at each grid point
D = one  # Damping coefficients
M = None # Coefficient related to modulus
L = None # Coefficient related to lightness

damper = 8 # damper size
d = 1.0

for k in reversed(range(damper)):
	d *= 1 - 1.0 / damper**2
	for j in range(ds[0]):
		D[k,j] *= d
		i = ds[1] - 1 - k
		D[i,j] *= d
	for i in range(ds[1]):
		D[i,k] *= d
		j = ds[0] - 1 - k
		D[i,j] *= d
# Python transform 'D[] *= d' to 'D.get = D.set * d'

t = index(one,D2) / float(ds[1])
x = (index(one,D1) - ds[0]/2.0) / (ds[0]/2.0)
c1 = t < 0.3
c2 = abs(t-0.7+0.2*exp(-8*x*x)+0.025*x) <= 0.1
M = con(c1,0.125,con(c2,0.5,0.3))
L = con(c1,0.125,con(c2,0.6,0.4))

pcount = 8 # Pulse counter
ptime  = 8 # Pulse time
pc = [ds[0]/3,ds[1]] # Pulse coordinate
N = 16 # Number of iterations

## Computation

for i in range(N):
	# Update pulse
	if (pcount > 0) :
		t = (pcount - ptime) / 2.0 * 0.05
		V[pc] += 64 * sqrt(M[pc]) * exp(-t*t)
		pcount--
	# Update stress
	S = S + M * (V(1,0) - V)
	T = S + M * (V(0,1) - V)
	# Update velocity
	V = D * (V + L * (S - S(-1,0) + T - T(0,-1)))

write(V, file)
