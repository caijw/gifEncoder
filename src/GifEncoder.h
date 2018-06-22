#ifndef GIFENCODER_H
#define GIFENCODER_H

#include <vector>
#include <stdint.h>
#include "ImageBuffer.h"

class GifEncoder {

    public:

        GifEncoder(int repeat, int delay, int sample);

        ~GifEncoder();

        void setDelay(int milliseconds);

        void setFrameRate(int fps);

        void setDispose(int disposalCode);

        void setRepeat(int repeat);

        void setTransparent(int color);

        void addFrame(ImageBuffer imageBuffer);

        void addFramesSyncLinear(std::vector<ImageBuffer> &imageBufferVec);

        void addFramesParallel(std::vector<ImageBuffer> &imageBufferVec);

        void finish();

        void setQuality(int quality);

        void start();

        std::vector<unsigned char>* out;

    private:

        int width;
        int height;
        int transparent;
        int transIndex;
        int repeat;
        int delay;
        unsigned char *image;
        std::vector<unsigned char> pixels;
        std::vector<unsigned char> indexedPixels;
        int colorDepth;
        std::vector<unsigned char> colorTab;
        std::vector<bool> usedEntry;
        int palSize; // color table size (bits-1)
        int dispose; // disposal code (-1 = use default)

        int sample; // default sample interval for quantizer

        bool started; // started encoding

        bool firstFrame;

};

#endif //GIFENCODER_H
