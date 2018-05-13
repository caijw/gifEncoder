#include "ImageBuffer.h"

ImageBuffer::ImageBuffer(unsigned char *buffer, unsigned int length, unsigned int index): buffer(buffer), length(length), index(index){

}

ImageBuffer::~ImageBuffer(){
	// delete []buffer;
}