#ifndef GIFFRAMEENCODER_H
#define GIFFRAMEENCODER_H

#include <vector>
#include <stdint.h>

class GifFrameEncoder
{
	public:
		GifFrameEncoder(
			unsigned char *imageData, 
			int channels, 
			int width, 
			int height, 
			int sample, 
			bool firstFrame, 
			int repeat,
			int transparent,
			int dispose,
			int delay);

		~GifFrameEncoder();

		std::vector<unsigned char>* out;

	private:
		unsigned char *image;

		int width;

		int height;

		std::vector<unsigned char> pixels;

		std::vector<unsigned char> indexedPixels;

		std::vector<bool> usedEntry;

		int sample;

		std::vector<unsigned char> colorTab;

		int colorDepth;

		int palSize;

		int transparent;

		int transIndex;

		bool firstFrame;

		int repeat;

		int dispose;

		int delay;

		void getImagePixels(int channels);

		void analyzePixels(int channels);

		void writeLSD();

		void writePalette();

		void writeNetscapeExt();

		void writeGraphicCtrlExt();

		void writeImageDesc();

		int GifFrameEncoder::findClosest(int c);

		void writeShort(int pValue);

		void writePixels();
};

#endif