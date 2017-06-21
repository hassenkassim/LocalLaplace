#ifndef IMAGE_HPP
#define IMAGE_HPP

using namespace std;


struct SubRegion { int r; int x; int y; };


class Image {
	
private:
	int width;
	int height;
	int offsetL;
	int offsetR;
	int offsetT;
	float *data;
	int size;

	void swap(Image &s);

public:
	Image() { }
	Image(int width, int height, const float *data);
	Image(int width, int height);
	Image(const Image *copy);
	Image(Image const &copy);
	~Image();
	Image & operator=(Image rhs) { rhs.swap(*this); return *this; }
	
	static Image *getScaledImage(int width, int height, const float *data);
	
	int getWidth() const;
	int getHeight() const;
	float *getData() const;
	int getSize() const;
	int getOffsetL() const { return offsetL; }
	int getOffsetR() const { return offsetR; }
	int getOffsetT() const { return offsetT; }
	
	float getPixel(int y, int x) const;
	void setPixel(int y, int x, float val);
	Image *getSubRegion(struct SubRegion *R0);
	void cutBorders(bool left, bool right, bool top, bool bottom);
	
	Image *copy() const;
	
    // debug
	void print() const;
	void saveImage(const char *fileName) const;
};


class ImageRGB {
	
private:
	int width;
	int height;
	unsigned char *dataRGB;
	float *dataYUV;
	int size;
	static unsigned char info[54];
	
public:
	ImageRGB(const char *filePath);
	~ImageRGB();
	
	int getWidth() const;
	int getHeight() const;
	int getSize() const;
	
	Image *getYChannel() const;
	void setYChannel(Image *Y);
	
	void saveImage(const char *fileName) const;
	
	static unsigned char *getInfoHeader();
};


#endif
