#include <node_api.h>
#include "common.h"
#include <iostream>
#include <vector>
#include "GifEncoder.h"
#include "ImageBuffer.h"

/*
    delay(Number), repeat(Number), quality(Number), buffers(Array of Buffer), parallel(Bool), callback(Function)
*/
napi_value picsToGIF(napi_env env, napi_callback_info info) {

    size_t argc = 6;
    napi_value args[6];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    NAPI_ASSERT(env, argc == 6, "gifEncoder picsToGIF: picsToGIF(delay, repeat, quality, buffers, parallel, callback), Wrong number of arguments. Expects 6 argument");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "gifEncoder picsToGIF: Wrong type of arguments. Expects a number as first argument delay.");

    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "gifEncoder picsToGIF: Wrong type of arguments. Expects a boolean as second argument repeat.");

    NAPI_CALL(env, napi_typeof(env, args[2], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "gifEncoder picsToGIF: Wrong type of arguments. Expects a number as third argument quality.");
    /*check Array of Buffer*/
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, args[3], &isArray));
    NAPI_ASSERT(env, isArray, "gifEncoder picsToGIF: Wrong type of arguments. Expects a array as 4th argument buffers.");
    uint32_t arrayLen = 0;
    bool isBuffer = false;
    NAPI_CALL(env, napi_get_array_length(env, args[3], &arrayLen));
    for(uint32_t i = 0; i < arrayLen; ++i){
        napi_value buffersEle;
        NAPI_CALL(env, napi_get_element(env, args[3], i, &buffersEle));
        NAPI_CALL(env, napi_is_buffer(env, buffersEle, &isBuffer));
        NAPI_ASSERT(env, isBuffer, "gifEncoder picsToGIF: Wrong type of arguments. Elements of buffers should be buffer type.");
    }

    NAPI_CALL(env, napi_typeof(env, args[4], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_boolean, "gifEncoder picsToGIF: Wrong type of arguments. Expects a boolean as 5th argument parallel.");

    NAPI_CALL(env, napi_typeof(env, args[5], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "gifEncoder picsToGIF: Wrong type of arguments. Expects a function as 6th argument callback.");

    int delay;
    int repeat;
    int quality;
    std::vector<ImageBuffer> imageBufferVec;
    bool parallel;
    napi_ref callback_ref;

    NAPI_CALL(env, napi_get_value_int32(env, args[0], &delay));
    NAPI_CALL(env, napi_get_value_int32(env, args[1], &repeat));
    NAPI_CALL(env, napi_get_value_int32(env, args[2], &quality));

    for(uint32_t i = 0; i < arrayLen; ++i){
        napi_value buffersEle;
        NAPI_CALL(env, napi_get_element(env, args[3], i, &buffersEle));
        unsigned char *rowData;
        size_t rowDataLen;
        NAPI_CALL(env, napi_get_buffer_info(env, buffersEle, (void **)&rowData, &rowDataLen));
        imageBufferVec.push_back( ImageBuffer(rowData, rowDataLen, i) );
    }
    NAPI_CALL(env, napi_get_value_bool(env, args[4], &parallel));
    NAPI_CALL(env, napi_create_reference(env, args[5], 1, &callback_ref ) );

    GifEncoder *gifEncoder = new GifEncoder(repeat, delay, quality);
    gifEncoder->start();

    if(parallel){
        gifEncoder->addFramesParallel(env, imageBufferVec, callback_ref);
    }else{
        gifEncoder->addFramesLinear(env, imageBufferVec, callback_ref);
    }

    return nullptr;
}

napi_value Init(napi_env env, napi_value exports) {

    napi_property_attributes attr = static_cast<napi_property_attributes>(napi_default | napi_writable | napi_enumerable | napi_configurable);

    napi_property_descriptor properties[] = {
      {"picsToGIF", 0, picsToGIF, 0, 0, 0, attr, 0 }
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties)/sizeof(*properties), properties));
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
