from map import * ## "Parallel Map Algebra" package

setupDevices("",0x04,"") # 0x04 == GPU

# Black Scholes, source ArrayFire

def cnd(x):
    temp = (x > 0)
    lhs = temp * (0.5 + blockerf(x/sqrt2)/2)
    rhs = (1 - temp) * (0.5 - erf((-x)/sqrt2)/2)
    return lhs + rhs

def black_scholes(S, X, R, V, T):
    S = randu(M, 1) # Underlying stock price
    X = randu(M, 1) # Strike Price
    R = randu(M, 1) # Risk free rate of interest
    V = randu(M, 1) # Volatility
    T = randu(M, 1) # Time to maturity

    d1 = log(S / X)
    d1 = d1 + (R + (V * V) * 0.5) * T
    d1 = d1 / (V * sqrt(T))

    d2 = d1 - (V * sqrt(T))
    cnd_d1 = cnd(d1)
    cnd_d2 = cnd(d2)

    C = S * cnd_d1 - (X * exp((-R) * T) * cnd_d2)
    P = X * exp((-R) * T) * (1 - cnd_d2) - (S * (1 -cnd_d1))

    return (C, P)


(C, P) = black_scholes(S, X, R, V, T)
print value(C)
#eval(C,P)
