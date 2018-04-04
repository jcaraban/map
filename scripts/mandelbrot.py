from map import * ## "Map Algebra Compiler" package
import sys

setupDevices("",0x04,"") # 0x04 == GPU

## 

N = 500
ds = [1024,1024]
xl = [-0.748766713922161, -0.748766707771757]
yl = [ 0.123640844894862,  0.123640851045266]

x = linspace( xlim(1), xlim(2), gridSize );
y = linspace( ylim(1), ylim(2), gridSize );

zr = xGrid + 1i*yGrid;
count = ones( size(z0) );

% Calculate
z = z0;
for n = 0:maxIterations
	temp = z.real * z.real - z.imag * z.imag + c.real
	imag = 2*real*imag + c.imag
	real = temp

	cr = rmin + y*sr
	ci = imin + x*si
	color = calc_mandel(c)

    z = z.*z + z0;
    inside = abs( z )<=2;
    count = count + inside;
end

## numba

@jit
def mandel(x, y, max_iters):
    """
    Given the real and imaginary parts of a complex number,
    determine if it is a candidate for membership in the Mandelbrot
    set given a fixed number of iterations.
    """
    i = 0
    c = complex(x, y)
    z = 0.0j
    for i in range(max_iters):
        z = z * z + c
        if (z.real * z.real + z.imag * z.imag) >= 4:
            return i

    return 255

@jit
def create_fractal(min_x, max_x, min_y, max_y, image, iters):
    height = image.shape[0]
    width = image.shape[1]

    pixel_size_x = (max_x - min_x) / width
    pixel_size_y = (max_y - min_y) / height
    for x in range(width):
        real = min_x + x * pixel_size_x
        for y in range(height):
            imag = min_y + y * pixel_size_y
            color = mandel(real, imag, iters)
            image[y, x] = color

    return image