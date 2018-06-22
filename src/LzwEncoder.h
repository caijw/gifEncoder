#ifndef LZWENCODER_H
#define LZWENCODER_H
#include <stdint.h>
#include <vector>



class LzwEncoder {

public:

    LzwEncoder(int width, int height, std::vector<uint8_t> &pixels, int colorDepth);

    ~LzwEncoder();

    // void encode(std::vector<uint8_t> &content);

    void encode(std::vector<uint8_t> *content);

private:

    int width;
    int height;
    std::vector<uint8_t> pixels;
    int colorDepth;



    int initCodeSize;

    uint8_t *accum;
    int32_t *htab;
    int32_t *codetab;

    int cur_accum;
    int cur_bits;
    int a_count;
    int free_ent;
    int maxcode;

    bool clear_flg;

    int g_init_bits;
    int ClearCode;
    int EOFCode;

    void char_out(uint8_t c, std::vector<uint8_t> *outs);

    void cl_block(std::vector<uint8_t> *outs);

    void cl_hash(int hsize);

    void compress(int init_bits, std::vector<uint8_t> *outs);

    void flush_char(std::vector<uint8_t> *outs);

    int MAXCODE(int n_bits);

    int nextPixel();

    void output(int code, std::vector<uint8_t> *outs);

    int n_bits;

    int remaining;

    int curPixel;
};



#endif