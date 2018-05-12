#include <vector>
#include <thread>
#include "GifEncoder.h"
#include "NeuQuant.h"
#include "LzwEncoder.h"
#include "Logger.h"
#include "GifFrameEncoder.h"


using namespace std;


GifEncoder::GifEncoder(int width,int height, int repeat, int delay, int sample){

    this->width = width;
    this->height = height;
    // transparent color if given
    this->transparent = -1;
    // transparent index in color table
    this->transIndex = 0;
    // -1 = no repeat, 0 = forever. anything else is repeat count
    this->repeat = repeat;
    // frame delay (hundredths)
    this->delay = delay;

    this->image = nullptr; // current frame

    this->pixels; // BGR byte array from frame

    this->indexedPixels; // converted frame indexed to palette

    this->colorDepth = 0; // number of bit planes

    this->colorTab; // RGB palette

    this->usedEntry; // active palette entries

    this->palSize = 7; // color table size (bits-1)

    this->dispose = -1; // disposal code (-1 = use default)

    this->firstFrame = true;

    this->sample = sample; // default sample interval for quantizer

    this->started = false;  // started encoding

    this->out = new std::vector<unsigned char>();

}


GifEncoder::~GifEncoder(){

}

void GifEncoder::setDelay(int milliseconds){
    this->delay = (int)std::round(milliseconds / 10);
}

void GifEncoder::setFrameRate(int fps){
    this->delay = (int)std::round(100 / fps);
}

void GifEncoder::setDispose(int disposalCode){
    if (disposalCode >= 0) this->dispose = disposalCode;
}

void GifEncoder::setRepeat(int repeat){
    this->repeat = repeat;
}

void GifEncoder::setTransparent(int color){
    this->transparent = color;
}

void GifEncoder::addFrame(unsigned char *imageData, int channels){

    this->image = imageData;


    this->getImagePixels(channels); // convert to correct format if necessary

    this->analyzePixels(channels); // build color table & map pixels

    if (this->firstFrame) {
      this->writeLSD(); // logical screen descriptior
      this->writePalette(); // global color table
      if (this->repeat >= 0) {
        // use NS app extension to indicate reps
        this->writeNetscapeExt();
      }
    }

    this->writeGraphicCtrlExt(); // write graphic control extension
    this->writeImageDesc(); // image descriptor
    if (!this->firstFrame) this->writePalette(); // local color table
    //this->writePalette();
    this->writePixels(); // encode and write pixel data

    this->firstFrame = false;

}

void GifEncoder::addFrames(std::vector<unsigned char *> &buffers,int channels){
    
    std::vector< std::vector<unsigned char> * > results;
    for(std::vector<unsigned char *>::size_type i = 0; i < buffers.size(); ++i){
        GifFrameEncoder frameEncoder(buffers[i], channels, this->width, this->height, this->sample, this->firstFrame, this->repeat, this->transparent, this->dispose);

        for(std::vector<unsigned char>::size_type j = 0; j < frameEncoder.out->size(); ++j){
            this->out->push_back(frameEncoder.out->at(j));
        }
        this->firstFrame = false;
    }

}


// void GifEncoder::addFrames(std::vector<unsigned char *> &buffers,int channels){
//     std::vector<thread> workers;
//     std::vector< std::vector<unsigned char> * > results;

//     for(int i = 0; i < buffers.size(); ++i){

//         workers.push_back(std::thread( [this, &results, i, channels](){

//             // std::vector<unsigned char>* out;

//             // this->image = imageData;


//             this->getImagePixels(channels); // convert to correct format if necessary

//             this->analyzePixels(channels); // build color table & map pixels

//             if (this->firstFrame) {
//               this->writeLSD(); // logical screen descriptior
//               this->writePalette(); // global color table
//               if (this->repeat >= 0) {
//                 // use NS app extension to indicate reps
//                 this->writeNetscapeExt();
//               }
//             }

//             this->writeGraphicCtrlExt(); // write graphic control extension
//             this->writeImageDesc(); // image descriptor
//             if (!this->firstFrame) this->writePalette(); // local color table
//             //this->writePalette();
//             this->writePixels(); // encode and write pixel data

//             this->firstFrame = false;


//         }));
//     }

//     for(int i = 0; i < workers.size(); ++i){
//         workers[i].join();
//     }


// }



void GifEncoder::finish(){
    this->out->push_back(0x3b);
}

void GifEncoder::setQuality(int quality){
    if (quality < 1) quality = 1;
    this->sample = quality;
}

void GifEncoder::start(){
    this->out->push_back('G');
    this->out->push_back('I');
    this->out->push_back('F');
    this->out->push_back('8');
    this->out->push_back('9');
    this->out->push_back('a');
    this->started = true;
}

void GifEncoder::analyzePixels(int channels){
    int len = this->pixels.size();
    int nPix = len / 3;

    if(this->usedEntry.size() == 0){
        this->usedEntry = std::vector<bool>(nPix);
        for(int i = 0; i < this->usedEntry.size(); i++){
            this->usedEntry[i] = false;
        }
    }
    
    this->indexedPixels = std::vector<unsigned char>(nPix);
    for(int i = 0; i < this->indexedPixels.size(); i++){
        this->indexedPixels[i] = 0;
    }

    NeuQuant* imgq = new NeuQuant( this->pixels, this->sample);

    imgq->buildColormap(); // create reduced palette

    this->colorTab = imgq->getColormap(); 

    // map image pixels to new palette
    int k = 0;
    for (int j = 0; j < nPix; j++) {

        unsigned char b = this->pixels[k++] & 0xff;
        unsigned char g = this->pixels[k++] & 0xff;
        unsigned char r = this->pixels[k++] & 0xff;


        int index = imgq->lookupRGB(
            b,
            g,
            r
        );

        this->usedEntry[index] = true;
        this->indexedPixels[j] = index;
    }


    this->pixels.clear();
    std::vector<unsigned char>(this->pixels).swap(this->pixels);

    this->colorDepth = 8;
    this->palSize = 7;

    // get closest match to transparent color if specified
    if (this->transparent != -1) {

        this->transIndex = this->findClosest(this->transparent);

        // ensure that pixels with full transparency in the RGBA image are using the selected transparent color index in the indexed image.
        for (int pixelIndex = 0; pixelIndex < nPix; pixelIndex++) {
            if (this->image[pixelIndex * channels + (channels - 1)] == 0) {
                this->indexedPixels[pixelIndex] = this->transIndex;
            }
        }
    }

    delete imgq;

}


int GifEncoder::findClosest(int c) {
    if (this->colorTab.size() == 0) return -1;

    int r = (c & 0xFF0000) >> 16;
    int g = (c & 0x00FF00) >> 8;
    int b = (c & 0x0000FF);
    int minpos = 0;
    int dmin = 256 * 256 * 256;
    int len = this->colorTab.size();

    for (int i = 0; i < len;) {
        int index = i / 3;
        int dr = r - (this->colorTab[i++] & 0xff);
        int dg = g - (this->colorTab[i++] & 0xff);
        int db = b - (this->colorTab[i++] & 0xff);
        int d = dr * dr + dg * dg + db * db;
        if (this->usedEntry[index] && (d < dmin)) {
            dmin = d;
            minpos = index;
        }
    }

    return minpos;
}

void GifEncoder::getImagePixels(int channels){
    int w = this->width;
    int h = this->height;
    this->pixels = std::vector<unsigned char>(w * h * 3);

    for(int i = 0; i < this->pixels.size(); i++){
        this->pixels[i] = 0;
    }

    unsigned char * data = this->image;
    int count = 0;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int b = (i * w * channels) + j * channels;
            this->pixels[count++] = data[b];
            this->pixels[count++] = data[b + 1];
            this->pixels[count++] = data[b + 2];
        }

    }
}

void GifEncoder::writeGraphicCtrlExt(){
    this->out->push_back(0x21); // extension introducer
    this->out->push_back(0xf9); // GCE label
    this->out->push_back(4); // data block size

    int transp, disp;
    if (this->transparent == -1) {
        transp = 0;
        disp = 0; // dispose = no action
    }
    else {
        transp = 1;
        disp = 2; // force clear if using transparent color
    }

    if (this->dispose >= 0) {
        disp = this->dispose & 7; // user override
    }
    disp <<= 2;

    // packed fields
    this->out->push_back(
        0 | // 1:3 reserved
        disp | // 4:6 disposal
        0 | // 7 user input - 0 = none
        transp // 8 transparency flag
    );

    this->writeShort(this->delay); // delay x 1/100 sec
    this->out->push_back(this->transIndex); // transparent color index
    this->out->push_back(0); // block terminator
}


void GifEncoder::writeImageDesc(){
    this->out->push_back(0x2c); // image separator
    this->writeShort(0); // image position x,y = 0,0
    this->writeShort(0);
    this->writeShort(this->width); // image size
    this->writeShort(this->height);

    // packed fields
    if (this->firstFrame) {
        // no LCT - GCT is used for first (or only) frame
        this->out->push_back(0);
    }
    else {
        // specify normal LCT
        this->out->push_back(
            0x80 | // 1 local color table 1=yes
            0 | // 2 interlace - 0=no
            0 | // 3 sorted - 0=no
            0 | // 4-5 reserved
            this->palSize // 6-8 size of color table
        );
    }
}


void GifEncoder::writeLSD(){
    // logical screen size
    this->writeShort(this->width);
    this->writeShort(this->height);

    // packed fields
    this->out->push_back(
        0x80 | // 1 : global color table flag = 1 (gct used)
        0x70 | // 2-4 : color resolution = 7
        0x00 | // 5 : gct sort flag = 0
        this->palSize // 6-8 : gct size
    );

    this->out->push_back(0); // background color index
    this->out->push_back(0); // pixel aspect ratio - assume 1:1
}


void GifEncoder::writeNetscapeExt(){
    this->out->push_back(0x21); // extension introducer
    this->out->push_back(0xff); // app extension label
    this->out->push_back(11); // block size
    this->out->push_back('N'); // app id + auth code
    this->out->push_back('E'); // app id + auth code
    this->out->push_back('T'); // app id + auth code
    this->out->push_back('S'); // app id + auth code
    this->out->push_back('C'); // app id + auth code
    this->out->push_back('A'); // app id + auth code
    this->out->push_back('P'); // app id + auth code
    this->out->push_back('E'); // app id + auth code
    this->out->push_back('2'); // app id + auth code
    this->out->push_back('.'); // app id + auth code
    this->out->push_back('0'); // app id + auth code
    this->out->push_back(3); // sub-block size
    this->out->push_back(1); // loop sub-block id
    this->writeShort(this->repeat); // loop count (extra iterations, 0=repeat forever)
    this->out->push_back(0); // block terminator
}

void GifEncoder::writePalette(){
    int size = this->colorTab.size();
    for(int i = 0; i < size; i++){
        this->out->push_back(this->colorTab[i]);
    }
    int n = (3 * 256) - this->colorTab.size();
    for (int i = 0; i < n; i++)
        this->out->push_back(0);
}

void GifEncoder::writeShort(int pValue){
    this->out->push_back(pValue & 0xFF);
    this->out->push_back((pValue >> 8) & 0xFF);
}


void GifEncoder::writePixels(){

    LzwEncoder* enc = new LzwEncoder(this->width, this->height, this->indexedPixels, this->colorDepth);

    enc->encode(this->out);

    delete enc;
}