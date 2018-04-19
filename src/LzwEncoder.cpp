#include "LzwEncoder.h"
#include <iostream>
#include <cmath>
#include <algorithm>


static int	eof = -10000;
static int	BITS = 12;
static int	HSIZE = 5003; // 80% occupancy
static int	masks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F,
	                        0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
	                        0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

LzwEncoder::LzwEncoder(int width, int height, std::vector<uint8_t> &pixels, int colorDepth){

	n_bits = 0;

	remaining = 0;

	curPixel = 0;

	this->width = width;
	this->height = height;
	this->pixels = pixels;
	this->initCodeSize = std::max(2, colorDepth);

	accum = new uint8_t[256];
	htab = new int32_t[HSIZE];
	codetab = new int32_t[HSIZE];

	cur_bits = 0;

	free_ent = 0; // first unused entry

	clear_flg = false;

}

LzwEncoder::~LzwEncoder() {
	delete accum;
	delete htab;
	delete codetab;
}



void LzwEncoder::char_out(uint8_t c, std::vector<uint8_t> *outs){
	accum[a_count++] = c;
	if (a_count >= 254) flush_char(outs);
}

void LzwEncoder::cl_block(std::vector<uint8_t> *outs){
	cl_hash(HSIZE);
	free_ent = ClearCode + 2;
	clear_flg = true;
	output(ClearCode, outs);
}

void LzwEncoder::cl_hash(int hsize){
	for (int i = 0; i < hsize; ++i) htab[i] = -1;
}

void LzwEncoder::compress(int init_bits, std::vector<uint8_t> *outs){
	int fcode, c, i, ent, disp, hsize_reg, hshift;

	// Set up the globals: g_init_bits - initial number of bits
	g_init_bits = init_bits;

	// Set up the necessary values
	clear_flg = false;
	n_bits = g_init_bits;
	maxcode = MAXCODE(n_bits);

	ClearCode = 1 << (init_bits - 1);
	EOFCode = ClearCode + 1;
	free_ent = ClearCode + 2;

	a_count = 0; // clear packet

	ent = nextPixel();

	hshift = 0;

	for (fcode = HSIZE; fcode < 65536; fcode *= 2) ++hshift;
	hshift = 8 - hshift; // set hash code range bound
	hsize_reg = HSIZE;

	cl_hash(hsize_reg); // clear hash table

	output(ClearCode, outs);

	outer_loop: while ((c = nextPixel()) != eof) {

	  fcode = (c << BITS) + ent;
	  i = (c << hshift) ^ ent; // xor hashing
	  if (htab[i] == fcode) {
	    ent = codetab[i];
	    continue;
	  } else if (htab[i] >= 0) { // non-empty slot
	    disp = hsize_reg - i; // secondary hash (after G. Knott)
	    if (i == 0) disp = 1;
	    do {
	      if ((i -= disp) < 0) i += hsize_reg;
	      if (htab[i] == fcode) {
	        ent = codetab[i];
	        goto  outer_loop;
	      }
	    } while (htab[i] >= 0);
	  }
	  output(ent, outs);
	  ent = c;
	  if (free_ent < 1 << BITS) {
	    codetab[i] = free_ent++; // code -> hashtable
	    htab[i] = fcode;
	  } else {
	    cl_block(outs);
	  }
	}

	// Put out the final code.
	output(ent, outs);
	output(EOFCode, outs);
};


void LzwEncoder::encode(std::vector<uint8_t> *outs){
	outs->push_back(initCodeSize); // write "initial code size" byte
	remaining = width * height; // reset navigation variables
	curPixel = 0;
	compress(initCodeSize + 1, outs); // compress and write the pixel data
	outs->push_back(0); // write block terminator
}

void LzwEncoder::flush_char(std::vector<uint8_t> *outs){
	if (a_count > 0) {
	  outs->push_back(a_count);
	  for(int i = 0; i < a_count; i++){
	  	outs->push_back(accum[i]);
	  }
	  a_count = 0;
	}
};


int LzwEncoder::MAXCODE(int n_bits){
	return (1 << n_bits) - 1;
};


int LzwEncoder::nextPixel(){
	if (remaining == 0) return eof;
	--remaining;
	uint8_t pix = pixels[curPixel++];
	return pix & 0xff;
};

void LzwEncoder::output(int code, std::vector<uint8_t> *outs){
	cur_accum &= masks[cur_bits];

	if (cur_bits > 0) cur_accum |= (code << cur_bits);
	else cur_accum = code;

	cur_bits += n_bits;

	while (cur_bits >= 8) {
	  char_out((cur_accum & 0xff), outs);
	  cur_accum >>= 8;
	  cur_bits -= 8;
	}

	// If the next entry is going to be too big for the code size,
	// then increase it, if possible.
	if (free_ent > maxcode || clear_flg) {
	  if (clear_flg) {
	    maxcode = MAXCODE(n_bits = g_init_bits);
	    clear_flg = false;
	  } else {
	    ++n_bits;
	    if (n_bits == BITS) maxcode = 1 << BITS;
	    else maxcode = MAXCODE(n_bits);
	  }
	}

	if (code == EOFCode) {
	  // At eof, write the rest of the buffer.
	  while (cur_bits > 0) {
	    char_out((cur_accum & 0xff), outs);
	    cur_accum >>= 8;
	    cur_bits -= 8;
	  }
	  flush_char(outs);
	}

};
