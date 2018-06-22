#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <thread>
#include <iostream>
#include "GifEncoder.h"
#include "NeuQuant.h"
#include "LzwEncoder.h"
#include "GifFrameEncoder.h"
#include "ImageBuffer.h"
#include <node_api.h>


typedef struct {
  napi_ref _callback;
  napi_async_work _request;
  GifEncoder *gifEncoder;
  std::vector<ImageBuffer> imageBufferVec;
  ImageBuffer imageBuffer;
} carrier;

static void deleteCallback(napi_env env, void* data, void* finalize_hint) {
    // std::cout << "-------------------------------free out----------------------------------" << std::endl;
    std::vector<unsigned char>* out = (std::vector<unsigned char>*)(finalize_hint);
    delete out;
}


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

    this->colorDepth = 0; // number of bit planes

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
    this->delay = milliseconds / 10;
}

void GifEncoder::setFrameRate(int fps){
    this->delay = 100 / fps;
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

void Execute(napi_env env, void* data) {

    carrier* the_carrier = static_cast<carrier*>(data);

    for(std::vector<unsigned char *>::size_type i = 0; i < the_carrier->imageBufferVec.size(); ++i){
       
        bool firstFrame = (the_carrier->imageBufferVec[i].index == 0);

        int imgWidth = 0,
            imgHeight = 0,
            imgChannels = 0,
            desiredChannels = 0;

        unsigned char *pixelData = stbi_load_from_memory(the_carrier->imageBufferVec[i].buffer, the_carrier->imageBufferVec[i].length, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

        GifFrameEncoder frameEncoder(pixelData, imgChannels, imgWidth, imgHeight, the_carrier->gifEncoder->sample, firstFrame, the_carrier->gifEncoder->repeat, the_carrier->gifEncoder->transparent, the_carrier->gifEncoder->dispose, the_carrier->gifEncoder->delay);

        the_carrier->gifEncoder->out->insert(the_carrier->gifEncoder->out->end(), frameEncoder.out->begin(), frameEncoder.out->end());

        the_carrier->gifEncoder->firstFrame = false;

        stbi_image_free(pixelData);

        delete frameEncoder.out;
    }

    the_carrier->gifEncoder->finish();
}

void Complete(napi_env env, napi_status status, void* data) {
    if (status != napi_ok) {
      napi_throw_type_error(env, nullptr, "Execute callback failed.");
      return;
    }

    carrier* the_carrier = static_cast<carrier*>(data);

    napi_value callback;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, the_carrier->_callback, &callback));
    napi_value argv[2];
    napi_value result;
    NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &argv[0]));
    unsigned char *bufData = the_carrier->gifEncoder->out->data();

    // NAPI_CALL(env,
    //           napi_create_external_buffer(
    //               env,
    //               3000000,
    //               theCopy,
    //               deleteTheText,
    //               NULL,  // finalize_hint
    //               &theBuffer));

    NAPI_CALL_RETURN_VOID(env, napi_create_external_buffer(env, the_carrier->gifEncoder->out->size(), bufData, deleteCallback, (void *)the_carrier->gifEncoder->out, &argv[1]) );
    // NAPI_CALL_RETURN_VOID(env, napi_create_buffer_copy(env, this->out->size(), (void *)(data), NULL, &argv[1]) );
    // delete this->out;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, callback, callback, 2, argv, &result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, the_carrier->_callback));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, the_carrier->_request));
    delete the_carrier;
}


void GifEncoder::addFramesLinear(napi_env env, std::vector<ImageBuffer> &imageBufferVec, napi_ref callback_ref){

    // this->imageBufferVec = imageBufferVec;

    napi_value resource_name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "GenerateGif", NAPI_AUTO_LENGTH, &resource_name));
    carrier *the_carrier = new carrier();

    the_carrier->_callback = callback_ref;

    the_carrier->gifEncoder = this;

    the_carrier->imageBufferVec = imageBufferVec;

    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource_name, Execute, Complete, the_carrier, &(the_carrier->_request) ));

    NAPI_CALL_RETURN_VOID(env, napi_queue_async_work(env, the_carrier->_request ));

}


void GifEncoder::addFramesParallel(napi_env env, std::vector<ImageBuffer> &imageBufferVec, napi_ref &callback_ref){

    // std::vector<std::thread> workers;

    // std::vector< std::vector<unsigned char> *> results( imageBufferVec.size(), nullptr);

    // for(std::vector<unsigned char *>::size_type i = 0; i < imageBufferVec.size(); ++i){

    //     workers.emplace_back([this, &results, i](std::vector<ImageBuffer> &imageBufferVec){

    //         bool firstFrame = (imageBufferVec[i].index == 0);

    //         int imgWidth = 0,
    //             imgHeight = 0,
    //             imgChannels = 0,
    //             desiredChannels = 0;

    //         unsigned char *pixelData = stbi_load_from_memory(imageBufferVec[i].buffer, imageBufferVec[i].length, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

    //         GifFrameEncoder frameEncoder(pixelData, imgChannels, imgWidth, imgHeight, this->sample, firstFrame, this->repeat, this->transparent, this->dispose, this->delay);

    //         results[i] = frameEncoder.out;

    //         stbi_image_free(pixelData);

    //     }, imageBufferVec);
    // }

    // for(std::vector<unsigned char *>::size_type i = 0; i < workers.size(); ++i){
    //     workers[i].join();
    // }

    // for(std::vector<unsigned char *>::size_type i = 0; i < results.size(); ++i){

    //     this->out->insert(this->out->end(), results[i]->begin(), results[i]->end());

    //     delete results[i];
    // }

    // this->finish();

    std::vector< std::vector<unsigned char> *> results( imageBufferVec.size(), nullptr);

    this->parallelEncodeResults = 

    napi_value resource_name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "GenerateGif", NAPI_AUTO_LENGTH, &resource_name));

    for(std::vector<unsigned char *>::size_type i = 0; i < imageBufferVec.size(); ++i){

        carrier *the_carrier = new carrier();

        the_carrier->_callback = callback_ref;

        the_carrier->gifEncoder = this;

        the_carrier->imageBuffer = imageBufferVec[i];

        NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource_name, Execute, Complete, the_carrier, &(the_carrier->_request) ));

        NAPI_CALL_RETURN_VOID(env, napi_queue_async_work(env, the_carrier->_request ));

    }



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