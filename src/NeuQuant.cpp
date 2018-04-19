/* NeuQuant Neural-Net Quantization Algorithm
* ------------------------------------------
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


#include <cstdint>
#include "NeuQuant.h"
#include <cmath>
#include <iostream>
#include "Logger.h"
#include <iomanip>
using namespace std;


NeuQuant::NeuQuant(std::vector<unsigned char> &thepic, int sample){
    pixels = thepic;
    lengthcount = thepic.size();
    samplefac = sample;
}
NeuQuant::~NeuQuant(){

}




/* Initialise network in range (0,0,0) to (255,255,255) and set parameters */
void NeuQuant::init() {
    
    // network = new pixel[netsize];
    // netindex = new int[netsize];
    // bias = new int[netsize];
    // freq = new int[netsize];
    // radpower = new int[initrad];

    for(int i = 0; i < netsize; i++){
        network[i][0] = network[i][1] = network[i][2] = network[i][3] = 0;
        netindex[i] = 0;
        bias[i] = 0;
        freq[i] = 0;
    }
    for(int i = 0; i < initrad; i++){
        radpower[i] = 0;
    }


    int i;
    double v;
    for (i = 0; i < netsize; i++) {
      v = (i << (netbiasshift + 8)) / netsize;
      network[i][0] = network[i][1] = network[i][2] = v;
      //network[i] = [v, v, v, 0]
      freq[i] = intbias / netsize;
      bias[i] = 0;
    }

}

/* Unbias network to give byte values 0..255 and record position i to prepare for sort */
void NeuQuant::unbiasnet() {

  for (int i = 0; i < netsize; i++) {
    network[i][0] = ((int)network[i][0]) >> netbiasshift;
    network[i][1] = ((int)network[i][1]) >> netbiasshift;
    network[i][2] = ((int)network[i][2]) >> netbiasshift;
    network[i][3] = i; // record color number
  }

}

/* Move neuron i towards biased (b,g,r) by factor alpha */
void NeuQuant::altersingle(double alpha, int i, int b, int g, int r) {

    network[i][0] -= (alpha * (network[i][0] - b)) / initalpha;
    network[i][1] -= (alpha * (network[i][1] - g)) / initalpha;
    network[i][2] -= (alpha * (network[i][2] - r)) / initalpha;

}

/* Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|] */
void NeuQuant::alterneigh(int radius, int i, int b, int g, int r) {

  int lo = std::abs(i - radius);
  int hi = std::min(i + radius, netsize);

  int j = i + 1;
  int k = i - 1;
  int m = 1;

  double *p;
  int a;
  while ((j < hi) || (k > lo)) {
  
    a = radpower[m++];

    if (j < hi) {
      p = network[j++];
      p[0] -= (a * (p[0] - b)) / alpharadbias;
      p[1] -= (a * (p[1] - g)) / alpharadbias;
      p[2] -= (a * (p[2] - r)) / alpharadbias;
    }

    if (k > lo) {
      p = network[k--];
      p[0] -= (a * (p[0] - b)) / alpharadbias;
      p[1] -= (a * (p[1] - g)) / alpharadbias;
      p[2] -= (a * (p[2] - r)) / alpharadbias;
    }
  }
}

/* Search for biased BGR values */
int NeuQuant::contest(int b, int g, int r) {
  /*
    finds closest neuron (min dist) and updates freq
    finds best neuron (min dist-bias) and returns position
    for frequently chosen neurons, freq[i] is high and bias[i] is negative
    bias[i] = gamma * ((1 / netsize) - freq[i])
  */

  double bestd = ~(1 << 31);
  double bestbiasd = bestd;
  int bestpos = -1;
  int bestbiaspos = bestpos;
  // console.log(b, g, r);
  int i;
  double *n;
  double dist;
  double biasdist;
  int betafreq;
  for (i = 0; i < netsize; i++) {
    // console.log(b, g, r);
    n = network[i];

    dist = std::abs(n[0] - b) + std::abs(n[1] - g) + std::abs(n[2] - r);
    if (dist < bestd) {
      bestd = dist;
      bestpos = i;
    }

    biasdist = dist - ((bias[i]) >> (intbiasshift - netbiasshift));
    if (biasdist < bestbiasd) {
      bestbiasd = biasdist;
      bestbiaspos = i;
    }

    betafreq = (freq[i] >> betashift);
    freq[i] -= betafreq;
    bias[i] += (betafreq << gammashift);

    // console.log("bestd: ", bestd, "bestbiasd:", bestbiasd, "bestpos:", bestpos, "bestbiaspos", bestbiaspos, "i:", i, "n:", n, "dist:", dist, "biasdist:", biasdist, "betafreq:", betafreq, "\n");


  }

  freq[bestpos] += beta;
  bias[bestpos] -= betagamma;

  return (int)bestbiaspos;

}

/* Insertion sort of network and building of netindex[0..255] (to do after unbias) */
void NeuQuant::inxbuild() {

    int i, j, smallpos, smallval, previouscol = 0, startpos = 0;
    double *p, *q;
    for (i = 0; i < netsize; i++) {

      p = network[i];
      smallpos = i;
      smallval = p[1]; // index on g
      // find smallest in i..netsize-1
      for (j = i + 1; j < netsize; j++) {
        q = network[j];
        if (q[1] < smallval) { // index on g
          smallpos = j;
          smallval = q[1]; // index on g
        }
      }
      q = network[smallpos];
      // swap p (i) and q (smallpos) entries
      if (i != smallpos) {
        j = q[0];   q[0] = p[0];   p[0] = j;
        j = q[1];   q[1] = p[1];   p[1] = j;
        j = q[2];   q[2] = p[2];   p[2] = j;
        j = q[3];   q[3] = p[3];   p[3] = j;
      }
      // smallval entry is now in position i

      if (smallval != previouscol) {
        netindex[previouscol] = (startpos + i) >> 1;
        for (j = previouscol + 1; j < smallval; j++)
          netindex[j] = i;
        previouscol = smallval;
        startpos = i;
      }
    }
    netindex[previouscol] = (startpos + maxnetpos) >> 1;
    for (j = previouscol + 1; j < 256; j++)
      netindex[j] = maxnetpos; // really 256

}

/* Search for BGR values 0..255 (after net is unbiased) and return colour index */
int NeuQuant::lookupRGB(int b, int g, int r) {
    double a, dist;
    double *p;
    double bestd = 1000; // biggest possible dist is 256*3
    double best = -1;

    int i = netindex[g]; // index on g
    int j = i - 1; // start at netindex[g] and work outwards

    while ((i < netsize) || (j >= 0)) {
      if (i < netsize) {
        p = network[i];
        dist = p[1] - g; // inx key
        if (dist >= bestd) i = netsize; // stop iter
        else {
          i++;
          if (dist < 0) dist = -dist;
          a = p[0] - b; if (a < 0) a = -a;
          dist += a;
          if (dist < bestd) {
            a = p[2] - r; if (a < 0) a = -a;
            dist += a;
            if (dist < bestd) {
              bestd = dist;
              best = p[3];
            }
          }
        }
      }
      if (j >= 0) {
        p = network[j];
        dist = g - p[1]; // inx key - reverse dif
        if (dist >= bestd) j = -1; // stop iter
        else {
          j--;
          if (dist < 0) dist = -dist;
          a = p[0] - b; if (a < 0) a = -a;
          dist += a;
          if (dist < bestd) {
            a = p[2] - r; if (a < 0) a = -a;
            dist += a;
            if (dist < bestd) {
              bestd = dist;
              best = p[3];
            }
          }
        }
      }
    }

    return best;
}

/* Main Learning Loop */
void NeuQuant::learn() {
    int i;

    int lengthcount = pixels.size();
    double alphadec = 30 + ((samplefac - 1) / 3); /*先用int*/
    double samplepixels = (double)lengthcount / (3 * samplefac);
    double delta = (samplepixels / ncycles);
    double alpha = initalpha;
    double radius = initradius;

    double rad = ((int)radius) >> radiusbiasshift;

    if (rad <= 1) rad = 0;
    for (i = 0; i < rad; i++){
      radpower[i] = alpha * (((rad * rad - i * i) * radbias) / (rad * rad));
    }

    int step;
    if (lengthcount < minpicturebytes) {
      samplefac = 1;
      step = 3;
    } else if ((lengthcount % prime1) != 0) {
      step = 3 * prime1;
    } else if ((lengthcount % prime2) != 0) {
      step = 3 * prime2;
    } else if ((lengthcount % prime3) != 0)  {
      step = 3 * prime3;
    } else {
      step = 3 * prime4;
    }

    int b, g, r, j;
    int pix = 0; // current pixel

    i = 0;
    while (i < samplepixels) {
      b = (pixels[pix] & 0xff) << netbiasshift;
      g = (pixels[pix + 1] & 0xff) << netbiasshift;
      r = (pixels[pix + 2] & 0xff) << netbiasshift;

      j = contest(b, g, r);

      altersingle(alpha, j, b, g, r);
      if (rad != 0) alterneigh(rad, j, b, g, r); // alter neighbours

      pix += step;
      if (pix >= lengthcount) pix -= lengthcount;

      i++;

      if (delta == 0) delta = 1;
      if (i % (int)delta == 0) {
        alpha -= alpha / alphadec;
        radius -= radius / radiusdec;
        rad = ((int)radius) >> radiusbiasshift;

        if (rad <= 1) rad = 0;
        for (j = 0; j < rad; j++)
          radpower[j] = alpha * (((rad * rad - j * j) * radbias) / (rad * rad));
      }
    }
}


void NeuQuant::buildColormap(){
    init();
    learn();
    unbiasnet();
    inxbuild();
};

std::vector<uint8_t> NeuQuant::getColormap(){
    std::vector<uint8_t> map;
    int * index = new int[netsize];

    for (int i = 0; i < netsize; i++)
      index[(int)network[i][3]] = i;

    int k = 0;
    for (int l = 0; l < netsize; l++) {
      int j = index[l];
      map.push_back((int)network[j][0] & 0xff);
      map.push_back((int)network[j][1] & 0xff);
      map.push_back((int)network[j][2] & 0xff);
    }
    delete index;
    
    return map;
};