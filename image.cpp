#include "image.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <FreeImage.h>
#include <omp.h>

#define SCALE_FACTOR 256

using namespace std;


// class Image

Image::Image(const int width, const int height, const float *data) {
	if (width < 0)
		cerr << "Image(int,int,float*): Argument width = "<< width <<"!" << endl;
	if (height < 0)
		cerr << "Image(int,int,float*): Argument height = "<< height <<"!" << endl;
    this->width = width;
    this->height = height;
    size = width * height;
    this->data = new float[size];
    for (int i = 0; i < size; i++) {
        this->data[i] = data[i];
    }
    offsetL = 0; offsetR = 0; offsetT = 0;
}

Image::Image(const int width, const int height) {
	if (width < 0)
		cerr << "Image(int,int): Argument width = "<< width <<"!" << endl;
	if (height < 0)
		cerr << "Image(int,int): Argument height = "<< height <<"!" << endl;
    this->width = width;
    this->height = height;
    size = width * height;
    this->data = new float[size];
    offsetL = 0; offsetR = 0; offsetT = 0;
}


Image::Image(const Image *copy) {
    width = copy->getWidth();
    height = copy->getHeight();
    size = copy->getSize();
    data = new float[size];
	
	float *copyData = copy->getData();
    for (int i = 0; i < size; i++) {
        this->data[i] = copyData[i];
    }
    offsetL = copy->getOffsetL();
    offsetR = copy->getOffsetR();
    offsetT = copy->getOffsetT();
}

Image::Image(Image const &copy) {
	width = copy.getWidth();
	height = copy.getHeight();
	size = copy.getSize();
	float *copyData = copy.data;
	
	for (int i = 0; i < size; i++)
		data[i] = copyData[i];
	
	offsetL = copy.offsetL;
	offsetR = copy.offsetR;
	offsetT = copy.offsetT;
}

Image::~Image() {
	delete [] data;
}

void Image::swap(Image &s) {
	int temp = width; width = s.width; s.width = temp;
	temp = height; height = s.height; s.height = temp;
	temp = size; size = s.size; s.size = temp;
	temp = offsetL; offsetL = s.offsetL; s.offsetL = temp;
	temp = offsetR; offsetR = s.offsetR; s.offsetR = temp;
	temp = offsetT; offsetT = s.offsetT; s.offsetT = temp;
	float *temp2 = data; data = s.data; s.data = temp2;
}

Image *Image::getScaledImage(const int width, const int height, const float *data) {
    int size = width * height;
    float *newData = new float[size];
    for (int i = 0; i < size; i++) newData[i] = data[i] / SCALE_FACTOR;
    Image *ret = new Image(width, height, newData);
    delete [] newData;
    return ret;
}

int Image::getWidth() const {
    return width;
}

int Image::getHeight() const {
    return height;
}

float *Image::getData() const {
    return data;
}

int Image::getSize() const {
    return size;
}

float Image::getPixel(int y, int x) const {
	y += offsetT; x += offsetL;

    // Adress pixels outside of image (border replication)
    if (y < offsetT) y = offsetT;
    else if (y >= height) y = height-1+offsetT;
    if (x < offsetL) x = offsetL;
    else if (x >= width) x = width-1+offsetL;
    
    return data[y*(width+offsetL+offsetR) + x];
}

void Image::setPixel(int y, int x, float val) {
	if (y < 0 || y >= (height) || y < 0 || x >= width)
		cerr << "Image::setPixel("<< y <<","<< x <<") out of bounds "<< height <<","<< width << endl;

	y += offsetT; x += offsetL;
	
    data[y*(width+offsetL+offsetR) + x] = val;
}

Image *Image::getSubRegion(struct SubRegion *R0) {
	int x = R0->x, y = R0->y, r = R0->r;

	// border locations in this image
	int xR = min(x+r, width-1), yB = min(y+r, height-1);
	int xL = x-r;
	if (xL < 0)  xL = 0; else R0->x = r;
	int yT = y-r;
	if (yT < 0)  yT = 0; else R0->y = r;

	// new dimensions
	const int newWidth  = xR+1 - xL;
	const int newHeight = yB+1 - yT;

	if (newWidth<0 || newHeight<0 || xL<0 || xR<0 || yT<0 || yB<0) {
		cerr << "Failure in Image::getSubRegion:" << endl;
		cerr << "Parameters: r = "<< r <<", x = "<< x <<", y = "<< y << endl;
		cerr << "Size of Image: "<< width <<" x "<< height << endl;
		cerr << "(xL, xR) = ("<< xL <<", "<< xR <<")" << endl;
		cerr << "(yT, yB) = ("<< yT <<", "<< yB <<")" << endl;
		cerr << "(newWidth, newHeight) = ("<< newWidth <<", "<< newHeight <<")" << endl;
	}

	// create sub-image
	Image *ret = new Image(newWidth, newHeight);
#ifdef _PARALLEL_
	#pragma omp parallel for
#endif
	for (int i = 0; i < newHeight; i++)
		for (int j = 0; j < newWidth; j++)
			ret->setPixel(i,j, getPixel(i+yT, j+xL));

	return ret;
}

void Image::cutBorders(bool left, bool right, bool top, bool bottom) {
	if (left) {
		offsetL++;
		width--;
	}
	if (right) {
		offsetR++;
		width--;
	}
	if (top) {
		offsetT++;
		height--;
	}
	if (bottom)
		height--;
}

Image *Image::copy() const {
    return new Image(width, height, data);
}

void Image::print() const {
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            cout << getPixel(r, c) << " ";
        }
        cout << endl;
    }
}

void Image::saveImage(const char *fileName) const {
/*	unsigned char *dataRGB = new unsigned char[size * 3];
	for (int i = 0; i < size; i++) {
		dataRGB[i*3]     = (unsigned char) (data[i] * SCALE_FACTOR);
		dataRGB[i*3 + 1] = (unsigned char) (data[i] * SCALE_FACTOR);
		dataRGB[i*3 + 2] = (unsigned char) (data[i] * SCALE_FACTOR);
	}
	
    FILE *f = fopen(fileName, "wb+");
    fwrite(ImageRGB::getInfoHeader(), sizeof(unsigned char), 54, f);
    fwrite(dataRGB, sizeof(unsigned char), size*3, f);
    fclose(f);*/
//	FIBITMAP *bitmap = FreeImage_Allocate(width, heigth, 24);
	unsigned char *dataGrey = new unsigned char[size];
	for (int i = 0; i < size; i++)
		dataGrey[i] = (BYTE) (data[i] * SCALE_FACTOR);
	FIBITMAP *bitmap = FreeImage_ConvertFromRawBits(dataGrey, width, height, width, 8, 0,0,0, false);
	FreeImage_Save(FIF_PNG, bitmap, fileName);
	FreeImage_Unload(bitmap);
	delete [] dataGrey;
}

/*Image &Image::operator=(const Image &a) {
	width = a.getWidth();
	height = a.getHeight();
	size = a.getSize();
	float *oldData = a.getData();
	for (int i = 0; i < size; i++)
		data[i] = oldData[i];
	return *this;
}
*/

// class ImageRGB

unsigned char ImageRGB::info[54];

ImageRGB::ImageRGB(const char *filePath) {
    
    FILE *f = fopen(filePath, "rb");
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    width = *(int*)&info[18];
    height = *(int*)&info[22];
    
    size = 3 * width * height;
    dataRGB = new unsigned char[size]; // allocate 3 bytes per pixel
    dataYUV = new float[size];
    fread(dataRGB, sizeof(unsigned char), size, f); // read the rest of the data at once
    fclose(f);
    
    for (int i = 0; i < size; i += 3) {
        unsigned char r = dataRGB[i];
        unsigned char g = dataRGB[i+1];
        unsigned char b = dataRGB[i+2];
        
        // convert to YUV color space
        float y = 0.299 * (float) r + 0.587 * (float) g + 0.144 * (float) b;
        float u = 0.493 * ((float) b - y);
        float v = 0.877 * ((float) r - y);
        
        dataYUV[i]   = y;
        dataYUV[i+1] = u;
        dataYUV[i+2] = v;
    }
}

ImageRGB::~ImageRGB() {
	delete [] dataRGB;
	delete [] dataYUV;
}
	
int ImageRGB::getWidth() const {
    return width;
}

int ImageRGB::getHeight() const {
    return height;
}

int ImageRGB::getSize() const {
    return size;
}

Image *ImageRGB::getYChannel() const {
    float *dataY = new float[size/3];
    for (int i = 0; i < size/3; i++)
        dataY[i] = dataYUV[i*3];
    Image *ret = Image::getScaledImage(width, height, dataY);
    delete [] dataY;
    return ret;
}

void ImageRGB::setYChannel(Image *Y) {
//	cout << "ImageRGB::setYChannel()" <<endl;
    float *dataY = Y->getData();
//	cout << " - Y->getData()" <<endl;
    for (int i = 0; i < size; i += 3)
        dataYUV[i] = dataY[i/3] * SCALE_FACTOR;
//	cout << " - Data copied into ImageRGB object" <<endl;
    
    for (int i = 0; i < size; i += 3) {
        float y = dataYUV[i];
        float u = dataYUV[i+1];
        float v = dataYUV[i+2];
        
        unsigned char r = (unsigned char) (y + v/0.877);
        unsigned char g = (unsigned char) (y - 0.39393 * u - 0.58081 * v);
        unsigned char b = (unsigned char) (y + u/0.493);
        
        dataRGB[i]   = r;
        dataRGB[i+1] = g;
        dataRGB[i+2] = b;
    }
}

void ImageRGB::saveImage(const char *fileName) const {
    FILE *f = fopen(fileName, "wb+");
    fwrite(info, sizeof(unsigned char), 54, f);
    fwrite(dataRGB, sizeof(unsigned char), size, f);
    fclose(f);
}

unsigned char *ImageRGB::getInfoHeader() {
		return info;
	}

