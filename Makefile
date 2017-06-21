CXXFLAGS =	-g -fmessage-length=0 -Wall

OBJS =		*.cpp

LIBOPTS =	-Ilibrary/include/ -Llibrary/lib/ -lfreeimage -fopenmp

TARGET =	LocalLaplacian

$(TARGET):	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBOPTS)

all:	$(TARGET)

parallel:	$(TARGET)
	mv LocalLaplacian LocalLaplacian_par

serial:	$(TARGET)
	mv LocalLaplacian LocalLaplacian_ser


clean:
	rm -f $(TARGET)
	rm -f blume_out.bmp *.png
	rm -f gaussPyramid/*.png
	
