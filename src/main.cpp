#include <node_api.h>
#include <iostream>
#include <vector>
#include "GifEncoder.h"
#include "ImageBuffer.h"

void buffer_delete_callback(char* data, void* the_vector) {
    delete reinterpret_cast<std::vector<unsigned char> *> (the_vector);
}

class GIFWorker : public Nan::AsyncWorker {
    public:
        GIFWorker(
            Nan::Callback * callback,
            std::vector<ImageBuffer> &imageBufferVec,
            int delay,
            int repeat,
            int parallel
        ) : Nan::AsyncWorker(callback), imageBufferVec(imageBufferVec), delay(delay), repeat(repeat), parallel(parallel)
        {

        	gifEncoder = new GifEncoder(repeat, delay, 10);
            gifEncoder->start();

        }

        ~GIFWorker(){
            delete gifEncoder;
            delete callback;
        }

        void Execute() {

            if(parallel){

                gifEncoder->addFramesParallel(imageBufferVec);

            }else{

                gifEncoder->addFramesSyncLinear(imageBufferVec);

            }

            gifEncoder->finish();

        }

        void HandleOKCallback () {

            Nan::HandleScope scope;

            v8::Local<v8::Object> gifBuffer = Nan::NewBuffer((char *)(gifEncoder->out->data()), gifEncoder->out->size(), buffer_delete_callback, gifEncoder->out).ToLocalChecked();

            v8::Local<v8::Value> argv[] = { Nan::Null(), gifBuffer };

            callback->Call(2, argv, async_resource);

        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::New(this->ErrorMessage()).ToLocalChecked(), // return error message
                Nan::Null()
            };
            callback->Call(2, argv);

        }

    private:
        int delay;
        int repeat;
        int parallel;
        GifEncoder *gifEncoder;
        std::vector<ImageBuffer> imageBufferVec;
};



NAN_METHOD(picsToGIF) {
    Nan::HandleScope scope;

    if (info.Length() != 5) {
        Nan::ThrowTypeError("Wrong number of arguments, expected 4 arguments!");
        return;
    }
    if (!info[0]->IsNumber()) {
        Nan::ThrowTypeError("Wrong arguments, delay should be a number!");
        return;
    }
    if (!info[1]->IsNumber()) {
        Nan::ThrowTypeError("Wrong arguments, repeat should be a number!");
        return;
    }

    /*judge array of buffer*/
    // if(!args[2]->()){
    //     isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, buffers should a Buffers type!")));
    //     return;
    // }

    if(!info[3]->IsNumber()){
        Nan::ThrowTypeError("Wrong arguments, parallel should be a number!");
        return;
    }

    if(!info[4]->IsFunction()){
        Nan::ThrowTypeError("Wrong arguments, callback should be a function!");
        return;
    }



    int delay = info[0]->Int32Value();
    int repeat = info[1]->Int32Value();
    v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(info[2]);
    int parallel = info[3]->Int32Value();
    Nan::Callback *callback = new Nan::Callback(Nan::To<v8::Function>(info[4]).ToLocalChecked());

    std::vector<ImageBuffer> imageBufferVec;
 
    for (unsigned int i = 0; i < array->Length(); ++i) {
        
        unsigned char *rowData = (unsigned char *)node::Buffer::Data( array->Get(i)->ToObject() );

        unsigned int rowDataLen = node::Buffer::Length(array->Get(i)->ToObject());

        imageBufferVec.push_back( ImageBuffer(rowData, rowDataLen, i) );

    }

    Nan::AsyncQueueWorker( new GIFWorker(callback, imageBufferVec, delay, repeat, parallel) );

}

/*
    delay(Number), repeat(Bool), quality(Number), buffers(Array of Buffer), parallel(Bool), callback(Function)
*/
void picsToGIF(napi_env env, napi_callback_info info) {

    size_t argc = 0;
    napi_value args[6];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    NAPI_ASSERT(env, argc == 6, "gifEncoder picsToGIF: picsToGIF(delay, repeat, quality, buffers, parallel, callback), Wrong number of arguments. Expects 6 argument");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "gifEncoder picsToGIF: Wrong type of arguments. Expects a number as first argument delay.");

    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_boolean, "gifEncoder picsToGIF: Wrong type of arguments. Expects a boolean as second argument repeat.");

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
    bool repeat;
    int quality;
    std::vector<ImageBuffer> imageBufferVec;
    bool parallel;
    napi_ref callback_ref;

    NAPI_CALL(env, napi_get_value_int32(env, args[0], &delay));
    NAPI_CALL(env, napi_get_value_bool(env, args[1], &repeat));
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
        gifEncoder->addFramesParallel(imageBufferVec, callback_ref);
    }else{
        gifEncoder->addFramesLinear(env, imageBufferVec, callback_ref);
    }
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



