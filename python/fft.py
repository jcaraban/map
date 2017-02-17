from map import * ## "Parallel Map Algebra" package

real = read('real')
imag = read('imaginary')

K = 10 # number of output frequencies
N = prod(real.datasize())
PI = 3.141592
out = []*K

for k in range(K) :
	angle = 2 * PI * index() / N
	oreal = zsum( real*cos(angle) + imag*sin(angle) )
	oimag = zsum(-real*sin(angle) + imag*cos(angle) )
	out[k].r = oreal
	out[k].i = oimag

print len(out)