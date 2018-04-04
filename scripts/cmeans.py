from map import * ## "Map Algebra Compiler" package

x1 = read('x1')
x2 = read('x2')
x3 = read('x3')
x4 = read('x4')
x5 = read('x5')
I = 100 # k-means iterations
C = 3   # number of clusters
S = prod(x1.datasize())

x = [x1,x2,x3,x4,x5] # vector of layers
w = [rand(x1) for i in range(C)]
c = [[rand(D0) for i in range(len(x))] for j in range(C)]

for i in range(I):
	dist = ones_like(x1) * float('inf')

	for j in range(C):
		c[j] = zsum(w[j]*x) / zsum(w[j])

	for j in range(C):
		norm = sqrt(localSum((x-c[j])**2))
		... continue ... how to compute w?
	

print c