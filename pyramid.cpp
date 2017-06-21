#include "pyramid.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "operations.hpp"

using namespace std;


// class Pyramid
	
int Pyramid::getN() const {
	return N;
}

Image *Pyramid::getLayer(const int i) const {
	return layers[i];
}

Image Pyramid::getUpsampledLayer(const int i) const {
	Image *temp1 = layers[i];
	for (int j = i; j > 0; j--) {
		Image *temp2 = new Image(layers[j-1]->getWidth(), layers[j-1]->getHeight());
		Operations::upsample(temp1, temp2);
		temp1 = temp2;
	}
	return Image(temp1);
}


// class GaussPyramid

GaussPyramid::GaussPyramid(const Image *Y, const int Nmax) {
	N = ceil(log2( min(Y->getWidth(), Y->getHeight()) )) + 1;
	if (Nmax != -1)
		N = min(N, Nmax);
	
	layers = new Image*[N];
	
	// Calculate Pyramid
	layers[0] = new Image(Y);
//	layers[0]->saveImage("gaussPyramid/layer0.png");
	
	for (int i = 1; i < N; i++) {
		int newWidth  = (layers[i-1]->getWidth()  + 1) / 2; // ceil(width / 2)
		int newHeight = (layers[i-1]->getHeight() + 1) / 2;
		layers[i] = new Image(newWidth, newHeight);
		
		Operations::downsample(layers[i-1], layers[i]);
		
//		char fileName[256];
//		sprintf(fileName, "gaussPyramid/layer%d.png", i);
//		layers[i]->saveImage(fileName);
	}
}

GaussPyramid::~GaussPyramid() {
	delete [] layers;
}


// class LaplacePyramid

LaplacePyramid::LaplacePyramid(const int width, const int height, const int Nmax) {
	N = ceil(log2( min(width, height) )) + 1;
	if (Nmax != -1)
		N = min(N, Nmax);
	
	layers = new Image*[N];
	
	// Calculate Pyramid
	layers[0] = new Image(width, height);
	for (int i = 1; i < N; i++) {
		int newWidth  = (layers[i-1]->getWidth()  + 1) / 2; // ceil(width / 2)
		int newHeight = (layers[i-1]->getHeight() + 1) / 2;
		layers[i] = new Image(newWidth, newHeight);
	}
}

LaplacePyramid::LaplacePyramid(LaplacePyramid const &copy) {
	N = copy.N;
	layers = new Image*[N];
	for (int i = 0; i < N; i++)
		layers[i] = new Image(copy.layers[i]);
}

LaplacePyramid::~LaplacePyramid() {
	delete [] layers;
}

Image *LaplacePyramid::collapse() {
//	cout << "collapse:" << endl;
    Image *temp1 = layers[N-1];
	for (int i = N-1; i > 0; i--) {
//		cout << "layer " << i << ": start" << endl;
		int newWidth  = layers[i-1]->getWidth();
		int newHeight = layers[i-1]->getHeight();
//		cout << "layer " << i << ": got new sizes" << endl;
		
		Image *temp2 = new Image(newWidth, newHeight);
//		cout << "layer " << i << ": created Image temp2" << endl;
		Operations::upsample(temp1, temp2);
//		cout << "layer " << i << ": did upsampling" << endl;
		temp1 = layers[i-1];
//		cout << "layer " << i << ": reassigned temp1 to layer[" << i-1<< "]" << endl;
		for (int r = 0; r < newHeight; r++)
			for (int c = 0; c < newWidth; c++)
				temp1->setPixel(r,c, temp1->getPixel(r,c) + temp2->getPixel(r,c));
		delete temp2;
//		cout << "layer " << i << ": calculated Pixels" << endl;
	}
	return new Image(temp1);
}
