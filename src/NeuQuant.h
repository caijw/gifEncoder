/* NeuQuant Neural-Net Quantization Algorithm Interface
 * ----------------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 */


#ifndef NEUQUANT_H
#define NEUQUANT_H
#include <vector>
#include <stdint.h>
#include <algorithm>

// For 256 colours, fixed arrays need 8kb, plus space for the image
#define netsize                    256           // number of colours used

/* Four primes near 500 - assume no image has a length so large that it is divisible by all four primes */
#define prime1                     499
#define prime2                     491
#define prime3                     487
#define prime4                     503

#define minpicturebytes            (3*prime4)    // minimum size for input image

/* Network Definitions */
#define maxnetpos                  netsize - 1           // netsize - 1
#define netbiasshift               4             // bias for colour values
#define ncycles                    100           // no. of learning cycles

/* Defs for freq and bias */
#define intbiasshift               16            // bias for fractions
#define intbias                    65536         // 1 << intbiasshift
#define gammashift                 10            // gamma = 1024
#define gamma                      1024          // 1 << gammashift
#define betashift                  10
#define beta                       64            // intbias >> betashift beta = 1/1024
#define betagamma                  65536         // intbias << (gammashift - betashift)

/* Defs for decreasing radius factor */
#define initrad                    32            // netsize >> 3 for 256 cols, radius starts
#define radiusbiasshift            6             // at 32.0 biased by 6 bits
#define radiusbias                 64            // 1 << radiusbiasshift
#define initradius                 2048          // initrad * radiusbias and decreases by a
#define radiusdec                  30            // factor of 1/30 each cycle

/* Defs for decreasing alpha factor */
#define alphabiasshift             10             // alpha starts at 1.0
#define initalpha                  1024           // 1 << alphabiasshift

/* Radbias and alpharadbias used for radpower calculation */
#define radbiasshift               8
#define radbias                    256            // 1 << radbiasshift
#define alpharadbshift             18             // alphabiasshift + radbiasshift
#define alpharadbias               262144


/* Program Skeleton
 * ----------------
 * [select samplefac in range 1..30]
 * pic = (unsigned char*) malloc(3*width*height);
 * [read image from input file into pic]
 *
 * initnet(pic,3*width*height,samplefac);
 * learn();
 * unbiasnet();
 *
 * [write output image header, using getColourMap(f),possibly editing the loops in that function]
 *
 * inxbuild();
 *
 * [write output image using inxsearch(b,g,r)]
 */
class NeuQuant {

public:

    NeuQuant(std::vector<unsigned char> &thepic, int sample);

    ~NeuQuant();

    // Initialise network in range (0,0,0) to (255,255,255) and set parameters
    void init();

    // Unbias network to give byte values 0..255 and record position i to prepare for sort */
    void unbiasnet();

    // Insertion sort of network and building of netindex[0..255] (to do after unbias)
    void inxbuild();

    // Search for BGR values 0..255 (after net is unbiased) and return colour index
    int lookupRGB(int b, int g, int r);

    // Main Learning Loop
    void learn();
    /*add func begin*/

    int contest(int b, int g, int r);

    void altersingle(double alpha, int i, int b, int g, int r);

    void alterneigh(int rad, int i, int b, int g, int r);

    void buildColormap();

    std::vector<uint8_t> getColormap();

    /*add func end*/
    int alphadec;                                     // biased by 10 bits

                                                      /* Types and Global Variables */
    std::vector<unsigned char> pixels;                // the input image itself
    int lengthcount;                          // lengthcount = H*W*3
    int samplefac;                            // sampling factor 1..30 */

    double network[netsize][4];                   // the network itself
    int netindex[netsize];                        // for network lookup - really 256
    int bias[netsize];                        // bias and freq arrays for learning
    int freq[netsize];                        // frequency array for learning
    int radpower[initrad];                    // radpower for precomputation

};

#endif 