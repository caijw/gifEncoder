#include <iostream>
#include <vector>
#include <v8.h>
#include <node.h>
#include <nan.h>
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


/*
    delay, repeat, buffers, callback, parallel
*/
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

NAN_MODULE_INIT(Init) {
        
    Nan::Set(
        target,
        Nan::New<v8::String>("picsToGIF").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(picsToGIF)).ToLocalChecked()
    );
}

NODE_MODULE(basic_nan, Init)