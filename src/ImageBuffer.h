#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

class ImageBuffer
{
public:
	ImageBuffer(unsigned char *buffer, unsigned int length, unsigned int index);
	~ImageBuffer();
	unsigned int length;
	unsigned char *buffer;
	unsigned int index;
};

#endif