
#ifndef GIFENCODER_H
#define GIFENCODER_H
#include <vector>
#include <iostream>
#include <stdint.h>

class GifEncoder {

    public:

        GifEncoder(int width,int height);

        void setDelay(int milliseconds);

        void setFrameRate(int fps);

        void setDispose(int disposalCode);

        void setRepeat(int repeat);

        void setTransparent(int color);

        void addFrame(unsigned char *imageData, int channels);

        void finish();

        void setQuality(int quality);

        void start();

        void analyzePixels(int channels);

        int findClosest(int c);

        void getImagePixels(int channels);

        void writeGraphicCtrlExt();

        void writeImageDesc();

        void writeLSD();

        void writeNetscapeExt();

        void writePalette();

        void writeShort(int);

        void writePixels();


        ~GifEncoder();
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

        std::vector<unsigned char>* out;


    private:

        bool firstFrame;

};

#endif //GIFENCODER_H
