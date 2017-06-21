#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

#include "image.hpp"
#include "pyramid.hpp"


class Operations {
	
public:
	// Filter Kernel for up-/downsampling
	static float w[5];
	
	static void upsample  (const Image *in, Image *out);
	static void downsample(const Image *in, Image *out);
	
	static float getLaplaceCoeff(const GaussPyramid *G, int i, int y, int x,
								 float g0, float sigmaR, float alpha);
	static void remap(Image *R0, float g0, float sigmaR, float alpha);
};


#endif
