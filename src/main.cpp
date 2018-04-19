
#include <iostream>
#include "GifEncoder.h"
#include <vector>
#include <node.h>
#include <nan.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Nan;
using namespace v8;
using namespace std;


void buffer_delete_callback(char* data, void* the_vector) {
    delete reinterpret_cast<std::vector<unsigned char> *> (the_vector);
}

class RGBAToGIFWorker : public Nan::AsyncWorker {
    public:
    RGBAToGIFWorker(Callback * callback, 
    				std::vector<unsigned char *> buffers,
    				int _width,
    				int _height,
                    int _channels,
    				int _delay,
                    int _repeat ) : Nan::AsyncWorker(callback) 
    {

    	buffersVec = buffers;
    	width = _width;
        channels = _channels;
    	height = _height;
    	delay = _delay;
        repeat = _repeat;

    	gifEncoder = new GifEncoder(width, height);
        gifEncoder->start();
        gifEncoder->setRepeat(repeat);
        gifEncoder->setDelay(delay);
        gifEncoder->setQuality(10);

    }
    void Execute() {

    	for (int i = 0; i < buffersVec.size(); i++) {
            gifEncoder->addFrame(buffersVec[i], channels);
    	}

        gifEncoder->finish();

    }
    void HandleOKCallback () {
        
        v8::Local<v8::Object> gifBuffer = Nan::NewBuffer((char *)(gifEncoder->out->data()), gifEncoder->out->size(), buffer_delete_callback, gifEncoder->out).ToLocalChecked();
        // v8::Local<v8::Object> gifBuffer = Nan::NewBuffer((char *)((gifEncoder->out).data()), (gifEncoder->out).size()).ToLocalChecked();
        // v8::Local<v8::Object> gifBuffer = Nan::CopyBuffer((char *)((gifEncoder->out).data()), (gifEncoder->out).size()).ToLocalChecked();
        v8::Local<v8::Value> argv[] = { Nan::Null(), gifBuffer };
        callback->Call(2, argv);
    }
    ~RGBAToGIFWorker(){
        delete gifEncoder;
        delete callback;
        for(int i = 0; i < buffersVec.size(); i++){
            delete buffersVec[i];
        }
    }

    private:
        std::vector<unsigned char *> buffersVec;
        int width;
        int height;
        int channels;
        int delay;
        int repeat;
        GifEncoder *gifEncoder = nullptr;
};



// /*
// 	delay, repeat, buffers, callback
// */
// void picsToGIF(const v8::FunctionCallbackInfo<Value>& args) {

// 	Isolate* isolate = args.GetIsolate();

//     if (args.Length() != 4) {
//         isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments, expected 4 arguments!")));
//         return;
//     }
//     if (!args[0]->IsNumber()) {
//       isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, delay should be a number!")));
//       return;
//     }
//     if (!args[1]->IsNumber()) {
//       isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, repeat should be a number!")));
//       return;
//     }

//     /*judge array of buffer*/
//     // if(!args[2]->()){
//     //     isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, buffers should a Buffers type!")));
//     //     return;
//     // }

//     if(!args[3]->IsFunction()){
//         isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, callback should be a function!")));
//         return;
//     }

// 	int delay = args[0]->Int32Value();
//     int repeat = args[1]->Int32Value();

// 	Local<Array> array = Local<Array>::Cast(args[2]);

// 	Nan::Callback *callback = new Nan::Callback( v8::Local<v8::Function>::Cast(args[3]));

// 	std::vector<unsigned char *> buffersVec;

//     int imgWidth = 0,
//         imgHeight = 0,
//         imgChannels = 0,
//         desiredChannels = 0,
//         rowDataLen = 0;

// 	for (unsigned int i = 0; i < array->Length(); i++ ) {



//         unsigned char *rowData = (unsigned char *)node::Buffer::Data( array->Get(i)->ToObject() );

//         rowDataLen = node::Buffer::Length(array->Get(i)->ToObject());

// 		unsigned char *pixelData = stbi_load_from_memory(rowData, rowDataLen, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

// 		buffersVec.push_back( pixelData );
// 	}

// 	Nan::AsyncQueueWorker(new RGBAToGIFWorker(callback, buffersVec, imgWidth, imgHeight, imgChannels, delay, repeat));

// }



/*
    delay, repeat, buffers, callback
*/
NAN_METHOD(picsToGIF) {
    Nan::HandleScope scope;

    if (info.Length() != 4) {
        Nan::ThrowTypeError("Wrong number of arguments, expected 4 arguments!");

        // isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments, expected 4 arguments!")));
        return;
    }
    if (!info[0]->IsNumber()) {
        Nan::ThrowTypeError("Wrong arguments, delay should be a number!");
        // isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, delay should be a number!")));
      return;
    }
    if (!info[1]->IsNumber()) {
        Nan::ThrowTypeError("Wrong arguments, repeat should be a number!");
        // isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, repeat should be a number!")));
      return;
    }

    /*judge array of buffer*/
    // if(!args[2]->()){
    //     isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, buffers should a Buffers type!")));
    //     return;
    // }

    if(!info[3]->IsFunction()){
        Nan::ThrowTypeError("Wrong arguments, callback should be a function!");
        // isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments, callback should be a function!")));
        return;
    }

    int delay = info[0]->Int32Value();
    int repeat = info[1]->Int32Value();

    Local<Array> array = Local<Array>::Cast(info[2]);

    Nan::Callback *callback =  new Callback(info[3].As<Function>());

    std::vector<unsigned char *> buffersVec;

    int imgWidth = 0,
        imgHeight = 0,
        imgChannels = 0,
        desiredChannels = 0,
        rowDataLen = 0;

    for (unsigned int i = 0; i < array->Length(); i++ ) {

        unsigned char *rowData = (unsigned char *)node::Buffer::Data( array->Get(i)->ToObject() );

        rowDataLen = node::Buffer::Length(array->Get(i)->ToObject());

        unsigned char *pixelData = stbi_load_from_memory(rowData, rowDataLen, &imgWidth, &imgHeight, &imgChannels, desiredChannels);

        buffersVec.push_back( pixelData );
    }

    Nan::AsyncQueueWorker(new RGBAToGIFWorker(callback, buffersVec, imgWidth, imgHeight, imgChannels, delay, repeat));

}






// void init(Local<Object> exports) {

//   NODE_SET_METHOD(exports, "picsToGIF", picsToGIF);



// }

// NODE_MODULE(NODE_GYP_MODULE_NAME, init)




NAN_MODULE_INIT(Init) {
        
   Nan::Set(target, New<String>("picsToGIF").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(picsToGIF)).ToLocalChecked());
}

NODE_MODULE(basic_nan, Init)