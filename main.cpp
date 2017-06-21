#include <iostream>
#include <FreeImage.h>

#include "image.hpp"
#include "pyramid.hpp"
#include "operations.hpp"


#include <cstdio>
#include <iostream>
#include <fstream>
#include <omp.h>

//#define _PARALLEL_


int main(int argc, const char **argv) {
	// Parameter
	float sigmaR = 0.3;
	float alpha = 0.35;
	
	FreeImage_Initialise();

	// RGB-Bild
	ImageRGB *I = new ImageRGB("autumn3840x2400.bmp");
	
	// Graustufen
	Image *Y = I->getYChannel();
	
	// Berechne Gauss-Pyramide
	GaussPyramid *G = new GaussPyramid(Y);
	
	// Initialisiere output Laplace-Pyramide
	LaplacePyramid *L = new LaplacePyramid(Y->getWidth(), Y->getHeight());
//	cout << "start" << endl;
	
	// Algorithmus
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
	for (int i = G->getN()-1; i >= 0; i--) {
		cout << "i = " << i << endl;
		Image *Gi = G->getLayer(i);
		Image *Li = L->getLayer(i);
#ifdef _PARALLEL_
		#pragma omp parallel for
#endif
		for (int y = 0; y < Gi->getHeight(); y++) {
			for (int x = 0; x < Gi->getWidth(); x++) {
				float g0 = Gi->getPixel(y,x);
				float l0 = Operations::getLaplaceCoeff(G, i,y,x, g0,sigmaR,alpha);
				Li->setPixel(y,x, l0);
			}
		}
	}
	
/*	for (int i = L->getN()-1; i >= 0; i--) {
		char fileName[256];
		sprintf(fileName, "laplacePyramid/layer%d.png", i);
		L->getLayer(i)->saveImage(fileName);
	}
*/
//	cout << "Size of Y: ("<< Y->getWidth() <<","<< Y->getHeight() <<")" << endl;
	Y = L->collapse();
//	cout << "L collapsed." << endl;
//	cout << "Size of Y: ("<< Y->getWidth() <<","<< Y->getHeight() <<")" << endl;
//	Y->saveImage("bla.png");
//	cout << "done test" << endl;
	
	// Update Bild, schreibe Datei
	I->setYChannel(Y);
//	cout << "Wrote Y back to I" << endl;
	I->saveImage("out.bmp");
	
	delete I;
	delete Y;
	delete G;
	delete L;
	FreeImage_DeInitialise();
	
	std::cout << "fertig" << std::endl;
}
