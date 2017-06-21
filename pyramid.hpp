#ifndef PYRAMID_HPP
#define PYRAMID_HPP

#include "image.hpp"


class Pyramid {
	
protected:
	int N;
	Image **layers;
	
public:
	int getN() const;
	Image *getLayer(int i) const;
	Image getUpsampledLayer(int i) const;
};


class GaussPyramid : public Pyramid {
	
public:
	GaussPyramid(const Image *Y, const int Nmax = -1);
	~GaussPyramid();
};


class LaplacePyramid : public Pyramid {
	
public:
	LaplacePyramid(int width, int height, const int Nmax = -1);
	LaplacePyramid(LaplacePyramid const &copy);
	~LaplacePyramid();
	
	Image *collapse();
};


#endif
