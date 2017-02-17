from map import * ## "Parallel Map Algebra" package

dem   = read("elevation")

fdire = flowDir8(dem)        # can be D8, FD8, FDD8, D-inf...
water = ones_like(fdir)
faccu = flowAccu(fdir,water) # or any other accumulation alg.
steep = sin(slope(dem))

a0 = 22.1 ; b0 = sin(radians(5.14)) ; m  = 0.4 ; n  = 1.3

R  = read("r_factor")
K  = read("k_factor")
LS = (m+1) * (faccu/a0)**m * (steep/b0)**n
C  = read("c_factor")
P  = read("p_factor")

A = R * K * LS * C * P

write(A,'rusle')

# RUSLE model: estimates the long-term annual average
# soil loss per unit area A (in tons/hectare/year)

# Ref: J.Sten et al “Parallel flow accumulation algorithms for
# graphical processing units with application to RUSLE model”
