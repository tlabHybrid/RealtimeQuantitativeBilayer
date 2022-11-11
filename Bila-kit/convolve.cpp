//
// 1D convolution program
// 

#include "MyHelper.h"

// This code provides 1d convolution with paddings for edge detection.
// X[SAMPLE_FREQ] : signal (the digitized current)
// h[]: filter (averaging + Prewitt)  [-1, -1, -1, ..., -1, 0, 1, ..., 1, 1, 1]
// prevX[500] : signal at previous timestep (for padding).
// Y[SAMPLE_FREQ] : filtered signal
const int kernel_size = 301;
//const int kernel_size = 101;
void convolve_EDGE(double* X, double* Y, double* prevX, int prevX_size) {
    
    // Filter preparation
    double H[kernel_size];
    for (int i = 0; i < (kernel_size - 1) / 2; i++) H[i] = -1;
    for (int i = (kernel_size - 1) / 2; i < (kernel_size + 1) / 2; i++) H[i] = 0;
    for (int i = (kernel_size + 1) / 2; i < kernel_size; i++) H[i] = 1;

    // Convolution
    for (int i = 0; i < SAMPLE_FREQ; i++) {
        Y[i] = 0;
        for (int j = 0; j < kernel_size; j++) {
            int tar = i + j - (kernel_size - 1) / 2;
            if (tar < 0) {
                Y[i] += prevX[prevX_size + tar] * H[j];
            }
            else if (tar >= SAMPLE_FREQ) {
                Y[i] += X[SAMPLE_FREQ-1] * H[j];
            }
            else {
                Y[i] += X[tar] * H[j];
            }
        }
        Y[i] /= ((double)kernel_size - 1);
    }
}

