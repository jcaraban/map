from map import * ## "Parallel Map Algebra" package

x1 = read('x1')
x2 = read('x2')
x3 = read('x3')
x4 = read('x4')
x5 = read('x5')
I = 100 # k-means iterations
K = 3   # number of clusters
S = prod(x1.datasize())

x = [x1,x2,x3,x4,x5] # vector of layers
c = [ [rand(D0) for i in range(len(x))] for k in range(K)]

for i in range(I) :
	dist = ones_like(x1) * float('inf')
	idx  = ones_like(x1) * -1
	for k in range(K) :
		norm = sqrt(localSum((x-c[k])**2))
		idx  = con(norm<dist,k,idx)
		dist = con(norm<dist,norm,dist)
	for k in range(K) :
		incl = (idx == k)
		c[k] = zonalSum(incl*x) / zonalSum(incl)

print c