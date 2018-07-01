#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <iostream>
#include "GifEncoder.h"
#include "NeuQuant.h"
#include "LzwEncoder.h"
#include "GifFrameEncoder.h"
#include "ImageBuffer.h"
#include <node_api.h>
#include <ctime>

typedef struct {
  napi_ref _callback;
  napi_async_work _request;
  GifEncoder *gifEncoder;
  std::vector<ImageBuffer> imageBufferVec;
} carrier;

static void deleteCallback(napi_env env, void* data, void* finalize_hint) {
    std::cout << "-------------------------------free out----------------------------------" << std::endl;
    std::vector<unsigned char>* out = (std::vector<unsigned char>*)(finalize_hint);
    delete out;
}

static long long currentTimeMillis() {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    auto timestamp = tmp.count();
    return timestamp;
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

    this->finishEncode = false;

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
    for( decltype(the_carrier->imageBufferVec.size()) i = 0; i < the_carrier->imageBufferVec.size(); ++i){
            long long currentTime = currentTimeMillis();
        bool firstFrame = (the_carrier->imageBufferVec[i].index == 0);
        int imgWidth = 0,
            imgHeight = 0,
            imgChannels = 0,
            desiredChannels = 0;

        unsigned char *pixelData = stbi_load_from_memory(the_carrier->imageBufferVec[i].buffer, the_carrier->imageBufferVec[i].length, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

        GifFrameEncoder frameEncoder(pixelData, imgChannels, imgWidth, imgHeight, the_carrier->gifEncoder->sample, firstFrame, the_carrier->gifEncoder->repeat, the_carrier->gifEncoder->transparent, the_carrier->gifEncoder->dispose, the_carrier->gifEncoder->delay);

        the_carrier->gifEncoder->encodeResults[the_carrier->imageBufferVec[i].index] = frameEncoder.out;

        the_carrier->gifEncoder->firstFrame = false;

        stbi_image_free(pixelData);
            long long diff = currentTimeMillis() - currentTime;
    std::cout << "Execute diff: " << diff << std::endl;
    }


}

void Complete(napi_env env, napi_status status, void* data) {

    if (status != napi_ok) {
      napi_throw_type_error(env, nullptr, "Execute callback failed.");
      return;
    }

    carrier* the_carrier = static_cast<carrier*>(data);
    for( decltype(the_carrier->gifEncoder->encodeResults.size()) i = 0; i < the_carrier->gifEncoder->encodeResults.size(); ++i ){
        if(the_carrier->gifEncoder->encodeResults[i] == nullptr){
            delete the_carrier;
            return;
        }
    }
    if(the_carrier->gifEncoder->finishEncode){
        delete the_carrier;
        return;
    }
    long long currentTime = currentTimeMillis();

    the_carrier->gifEncoder->finishEncode = true;

    for(decltype(the_carrier->gifEncoder->encodeResults.size()) i = 0; i < the_carrier->gifEncoder->encodeResults.size(); ++i){
        the_carrier->gifEncoder->out->insert(the_carrier->gifEncoder->out->end(), the_carrier->gifEncoder->encodeResults[i]->begin(), the_carrier->gifEncoder->encodeResults[i]->end());
        delete the_carrier->gifEncoder->encodeResults[i];
    }

    the_carrier->gifEncoder->finish();

    napi_value callback;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, the_carrier->_callback, &callback));
    napi_value argv[2];
    napi_value result;
    NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &argv[0]));
    unsigned char *bufData = the_carrier->gifEncoder->out->data();

    NAPI_CALL_RETURN_VOID(env, napi_create_external_buffer(env, the_carrier->gifEncoder->out->size(), bufData, deleteCallback, (void *)the_carrier->gifEncoder->out, &argv[1]) );
    int64_t adjust_external_memory_result;
    NAPI_CALL_RETURN_VOID(env, napi_adjust_external_memory(env, the_carrier->gifEncoder->out->size(), &adjust_external_memory_result)); //nodejs v10 should call this api to give v8 a hint to trigger gc.If no, memery will leak. see discussion https://github.com/nodejs/node/issues/21441

    long long diff = currentTimeMillis() - currentTime;
    std::cout << "Complete diff: " << diff << std::endl;

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, callback, callback, 2, argv, &result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, the_carrier->_callback));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, the_carrier->_request));
    delete the_carrier;
}


void GifEncoder::addFramesLinear(napi_env env, std::vector<ImageBuffer> &imageBufferVec, napi_ref callback_ref){
    for( decltype(imageBufferVec.size()) i = 0; i < imageBufferVec.size(); ++i ){
        this->encodeResults.push_back(nullptr);
    }
    napi_value resource_name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "GenerateGif", NAPI_AUTO_LENGTH, &resource_name));
    carrier *the_carrier = new carrier();
    the_carrier->_callback = callback_ref;
    the_carrier->gifEncoder = this;
    the_carrier->imageBufferVec = imageBufferVec;
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource_name, Execute, Complete, the_carrier, &(the_carrier->_request) ));
    NAPI_CALL_RETURN_VOID(env, napi_queue_async_work(env, the_carrier->_request ));

}


void GifEncoder::addFramesParallel(napi_env env, std::vector<ImageBuffer> &imageBufferVec, napi_ref callback_ref){

    for( decltype(imageBufferVec.size()) i = 0; i < imageBufferVec.size(); ++i ){
        this->encodeResults.push_back(nullptr);
    }

    napi_value resource_name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "GenerateGif", NAPI_AUTO_LENGTH, &resource_name));

    for(std::vector<unsigned char *>::size_type i = 0; i < imageBufferVec.size(); ++i){

        carrier *the_carrier = new carrier();

        the_carrier->_callback = callback_ref;

        the_carrier->gifEncoder = this;

        the_carrier->imageBufferVec.push_back( ImageBuffer(imageBufferVec[i].buffer, imageBufferVec[i].length, imageBufferVec[i].index) );

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