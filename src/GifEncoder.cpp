
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <node_api.h>
#include <vector>
#include <thread>
#include <iostream>
#include "GifEncoder.h"
#include "NeuQuant.h"
#include "LzwEncoder.h"
#include "Logger.h"
#include "GifFrameEncoder.h"
#include "ImageBuffer.h"




GifEncoder::GifEncoder(int repeat, int delay, int sample){

    // transparent color if given
    this->transparent = -1;
    // transparent index in color table
    this->transIndex = 0;
    // -1 = no repeat, 0 = forever. anything else is repeat count
    this->repeat = repeat;
    // frame delay (hundredths)
    this->delay = delay;

    this->image = nullptr; // current frame

    //this->pixels; // BGR byte array from frame

    //this->indexedPixels; // converted frame indexed to palette

    this->colorDepth = 0; // number of bit planes

    //this->colorTab; // RGB palette

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

void GifEncoder::addFrame(ImageBuffer imageBuffer){

    bool firstFrame = (imageBuffer.index == 0);

    int imgWidth = 0,
        imgHeight = 0,
        imgChannels = 0,
        desiredChannels = 0;

    unsigned char *pixelData = stbi_load_from_memory(imageBuffer.buffer, imageBuffer.length, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

    GifFrameEncoder frameEncoder(pixelData, imgChannels, imgWidth, imgHeight, this->sample, firstFrame, this->repeat, this->transparent, this->dispose, this->delay);

    this->out->insert(this->out->end(), frameEncoder.out->begin(), frameEncoder.out->end());

    stbi_image_free(pixelData);

    delete frameEncoder.out;

}

void GifEncoder::addFramesLinear(napi_env env, std::vector<ImageBuffer> &imageBufferVec, napi_ref callback_ref){

    for(std::vector<unsigned char *>::size_type i = 0; i < imageBufferVec.size(); ++i){
       
        bool firstFrame = (imageBufferVec[i].index == 0);

        int imgWidth = 0,
            imgHeight = 0,
            imgChannels = 0,
            desiredChannels = 0;

        unsigned char *pixelData = stbi_load_from_memory(imageBufferVec[i].buffer, imageBufferVec[i].length, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

        GifFrameEncoder frameEncoder(pixelData, imgChannels, imgWidth, imgHeight, this->sample, firstFrame, this->repeat, this->transparent, this->dispose, this->delay);

        this->out->insert(this->out->end(), frameEncoder.out->begin(), frameEncoder.out->end());

        this->firstFrame = false;

        stbi_image_free(pixelData);

        delete frameEncoder.out;
    }
    gifEncoder->finish();


    napi_value callback;
    NAPI_CALL(env, napi_get_reference_value(env, callback_ref, &callback));
    napi_value argv[2];
    napi_value result;
    NAPI_CALL(env, napi_get_null(env, &argv[0]));
    void ** data = (void **)( &( gifEncoder->out->data() ) );
    NAPI_CALL(env, napi_create_buffer(env, gifEncoder->out->size(), data, &argv[1]) );
    NAPI_CALL(env, napi_call_function(env, callback, callback, 2, argv, &result));

}


void GifEncoder::addFramesParallel(std::vector<ImageBuffer> &imageBufferVec, napi_ref &callback_ref){

    std::vector<std::thread> workers;

    std::vector< std::vector<unsigned char> *> results( imageBufferVec.size(), nullptr);

    for(std::vector<unsigned char *>::size_type i = 0; i < imageBufferVec.size(); ++i){

        workers.emplace_back([this, &results, i](std::vector<ImageBuffer> &imageBufferVec){

            bool firstFrame = (imageBufferVec[i].index == 0);

            int imgWidth = 0,
                imgHeight = 0,
                imgChannels = 0,
                desiredChannels = 0;

            unsigned char *pixelData = stbi_load_from_memory(imageBufferVec[i].buffer, imageBufferVec[i].length, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

            GifFrameEncoder frameEncoder(pixelData, imgChannels, imgWidth, imgHeight, this->sample, firstFrame, this->repeat, this->transparent, this->dispose, this->delay);

            results[i] = frameEncoder.out;

            stbi_image_free(pixelData);

        }, imageBufferVec);
    }

    for(std::vector<unsigned char *>::size_type i = 0; i < workers.size(); ++i){
        workers[i].join();
    }

    for(std::vector<unsigned char *>::size_type i = 0; i < results.size(); ++i){

        this->out->insert(this->out->end(), results[i]->begin(), results[i]->end());

        delete results[i];
    }

    gifEncoder->finish();

    v8::Local<v8::Object> gifBuffer = Nan::NewBuffer((char *)(gifEncoder->out->data()), gifEncoder->out->size(), buffer_delete_callback, gifEncoder->out).ToLocalChecked();

    v8::Local<v8::Value> argv[] = { Nan::Null(), gifBuffer };

    callback->Call(2, argv, async_resource);



}



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