#include "operations.hpp"

#include <omp.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>

using namespace std;


float Operations::w[] = {0.05, 0.25, 0.4, 0.25, 0.05};

void Operations::upsample(const Image *in, Image *out) {
    
    // Filter over rows
    Image *temp = new Image(in->getWidth(), out->getHeight());
	
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
    for (int c = 0; c < temp->getWidth(); c++) {
		for (int r = 0; r < temp->getHeight(); r+=2) { // "existing" pixels
            float val = w[0] * in->getPixel(0.5*r - 1, c)
					  + w[2] * in->getPixel(0.5*r,     c)
					  + w[4] * in->getPixel(0.5*r + 1, c);
            
            temp->setPixel(r, c, 2*val);
        }
		for (int r = 1; r < temp->getHeight(); r+=2) { // "new" pixels caused by doubling size
			float val = w[1] * in->getPixel(0.5*r,     c)
					  + w[3] * in->getPixel(0.5*r + 1, c);
				
			temp->setPixel(r, c, 2*val);
		}
	}
    
    // Filter over columns
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
    for (int r = 0; r < out->getHeight(); r++) {
        for (int c = 0; c < out->getWidth(); c+=2) { // "existing" pixels
            float val = w[0] * temp->getPixel(r, 0.5*c - 1)
					  + w[2] * temp->getPixel(r, 0.5*c)
					  + w[4] * temp->getPixel(r, 0.5*c + 1);
            
            out->setPixel(r, c, 2*val);
        }
        for (int c = 1; c < out->getWidth(); c+=2) { // "new" pixels
            float val = w[1] * temp->getPixel(r, 0.5*c)
					  + w[3] * temp->getPixel(r, 0.5*c + 1);
            
            out->setPixel(r, c, 2*val);
        }
    }
    
    delete temp;
}

void Operations::downsample(const Image *in, Image *out) {
	
    // Filter over rows
    Image *temp = new Image(in->getWidth(), out->getHeight());
	
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
    for (int r = 0; r < temp->getHeight(); r++) {
        for (int c = 0; c < temp->getWidth(); c++) {
			float val = w[0] * in->getPixel(2*r - 2, c)
                      + w[1] * in->getPixel(2*r - 1, c)
                      + w[2] * in->getPixel(2*r,     c)
                      + w[3] * in->getPixel(2*r + 1, c)
                      + w[4] * in->getPixel(2*r + 2, c);
            
            temp->setPixel(r, c, val);
        }
    }
    
    // Filter over columns
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
    for (int r = 0; r < out->getHeight(); r++) {
        for (int c = 0; c < out->getWidth(); c++) {
            float val = w[0] * temp->getPixel(r, 2*c - 2)
                      + w[1] * temp->getPixel(r, 2*c - 1)
                      + w[2] * temp->getPixel(r, 2*c)
                      + w[3] * temp->getPixel(r, 2*c + 1)
                      + w[4] * temp->getPixel(r, 2*c + 2);
            
            out->setPixel(r, c, val);
        }
    }
    
    delete temp;
}

float Operations::getLaplaceCoeff(const GaussPyramid *G, const int i, const int y,
        const int x, const float g0, const float sigmaR, const float alpha) {

	// maximum intermediate Laplace pyramid layers to be evaluated
	const int maxLayers = 5;

	// Top pixel
	if (i == G->getN()-1) {
		int i_R0 = max(0, i-maxLayers+2);
		Image *R0 = new Image(G->getLayer(i_R0));
		remap(R0, g0, sigmaR, alpha);
		GaussPyramid Gtemp(R0);
		float l0 = Gtemp.getLayer(Gtemp.getN()-1)->getPixel(1,1);
		delete R0;
		return l0;
	}

	// other Pixels...
	// pyramid-layer of R0 location
	int i_R0 = i-maxLayers+2;
	int difference;
	if (i_R0 < 0) {
		difference = -i_R0;
		i_R0 = 0;
	} else
		difference = 0;

	// number of L-layers to evaluate
	const int Nmax = maxLayers - difference;

	// calculate "radius" and center of R0 and layers above
	const int subLevels = Nmax - 2;

	struct SubRegion R[subLevels + 1];

	R[subLevels].r = 2; // radius from center pixel to border
	R[subLevels].x = x; // center x position
	R[subLevels].y = y; // center y position

	for (int n = subLevels-1; n >= 0; n--) {
		R[n].r = 2*R[n+1].r + 2;
		R[n].x = 2*R[n+1].x;
		R[n].y = 2*R[n+1].y;
	}

//	int x_old = R[0].x, y_old = R[0].y;

	// get R0
	Image *R0 = G->getLayer(i_R0)->getSubRegion( &(R[0]) );

	// a huge mess of debug output
//	if (R[0].x != x_old || R[0].y != y_old)
//		cout << "R0 center updated from ("<< y_old <<", "<< x_old <<") to ("<< R[0].y <<", "<< R[0].x <<"), r = "<< R[0].r << endl;

	// update center pixel locations
	for (int n = 1; n <= subLevels; n++) {
		R[n].x = 0.5 * R[n-1].x;
		R[n].y = 0.5 * R[n-1].y;
	}

//	cout << "L"<<i<<"("<<x<<","<<y<<") -> R0(["<<R[0].y-R[0].r<<":"<<R[0].y+R[0].r<<"],["<<R[0].y-R[0].r<<":"<<R[0].y+R[0].r<<"])" << endl;

	// apply remapping function
	remap(R0, g0, sigmaR, alpha);

	// build intermediate G-pyramid
	Image *Gn = new Image(R0);
	for (int n = 0; n < subLevels; n++) {
		int newWidth  = (Gn->getWidth()  + 1) / 2;
		int newHeight = (Gn->getHeight() + 1) / 2;
		Image *Gnp1 = new Image(newWidth, newHeight);

		downsample(Gn, Gnp1);

		// discard additionally replicated borders (l,r,t,b)
		bool left = (R[n].x-R[n].r) >= 0, right  = (R[n].x+R[n].r) < Gn->getWidth();
		bool top  = (R[n].y-R[n].r) >= 0, bottom = (R[n].y+R[n].r) < Gn->getHeight();
		Gnp1->cutBorders(left, right, top, bottom);

		delete Gn;
		Gn = Gnp1;
	}
	int n = Nmax - 1;

	// calculate top pixel manually (downsampling)
	float temp[5];
	for (int k = -2; k <= 2; k++)
		temp[k+2] += w[0] * Gn->getPixel(R[n].y + k, R[n].x - 2)
				   + w[1] * Gn->getPixel(R[n].y + k, R[n].x - 1)
				   + w[2] * Gn->getPixel(R[n].y + k, R[n].x)
				   + w[3] * Gn->getPixel(R[n].y + k, R[n].x + 1)
				   + w[4] * Gn->getPixel(R[n].y + k, R[n].x + 2);
	float gnp1 = w[0]*temp[0] + w[1]*temp[1] + w[2]*temp[2] + w[3]*temp[3] + w[4]*temp[4];

	// output Laplacian coefficient
	float l0 = Gn->getPixel(R[n].y, R[n].x) - gnp1;

	delete Gn;
	delete R0;
	return l0;
}

void Operations::remap(Image *R0, const float g0, const float sigmaR, const float alpha) {
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
	for (int y = 0; y < R0->getHeight(); y++) {
		for (int x = 0; x < R0->getWidth(); x++) {
			float diff = R0->getPixel(y,x) - g0;
			if (abs(diff) <= sigmaR) {
				float sign_diff = (float) ((diff > 0) - (diff < 0));
				R0->setPixel(y,x, g0 + sign_diff * sigmaR * pow(abs(diff)/sigmaR, alpha));
			}
		}
	}
}