## Lattice Boltzman Method D2Q9, inspired by Sébastien Leclaire (2014)
## who was inspired from Iain Haslam (http://exolete.com/lbm/)

from map import * ## "Parallel Map Algebra" package

o = 1.0 # omega
p = 1.0 # rho
w1, w2, w3 = 4.0/9, 1.0/9, 1.0/36
w = [w1,w2,w3,w2,w3,w2,w3,w2,w3]
cx = [0, 1, 1, 0,-1,-1,-1, 0, 1]
cy = [0, 0, 1, 1, 1, 0,-1,-1,-1]
ds = [1024,1024]
bs = [256,256]

s = rand(1234,ds,F32,ROW+BLK,bs) < 0.7 # solid
f = [!s*p*ones(ds,F32,ROW+BLK,bs)/9]*9 # fluid

N = 100 # number of time steps

def streaming(f):
	opp = [0 5 6 7 8 1 2 3 4] # opposite
	nf  = [0]*9 # new fluid velocities
	for i in Range(9):
		ngb = (-cx[i],-cy[i]) # neighbor
		forwa = f[i](ngb) * !s(ngb)
		bback = f[opp[i]] * s(ngb)
		nf[i] = (forwa + bback) * !s
	return nf

def feq(p,ux,uy):
	cu = cx*ux + cy*uy
	u2 = ux**2 + uy**2
	return p*w*(1+3*cu+4.5*cu*cu-1.5*u2)

def collision(f):
	p  = lsum(f)
	ux = con( lsum(f*cx)/p, p!=0, 0)
	uy = lsum(f*cy)/p if p!=0 else 0
	f  = (1-o)*f + o*feq(p,ux,uy)
	
for i in range(N):
	f  = streaming(f)
	f  = collision(f)

write(f,'output')
