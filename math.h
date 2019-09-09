//
// Created by chenjs on 19-6-14.
//

#ifndef IMAGEPROCESSOR6_MATH_H
#define IMAGEPROCESSOR6_MATH_H

#include "array.h"

enum class ConvolutionStrategy{

};

template <typename TPixel>
Array<TPixel, 2> convolution(Array<TPixel, 2> src, Array<TPixel, 2> kernel){
    size_t src_height = src.size(0);
    size_t src_width = src.size(1);
    size_t kernel_height = kernel.size(0);
    size_t kernel_width = kernel.size(1);
    size_t dst_height = src_height - kernel_height + 1;
    size_t dst_width = src_width - kernel_width + 1;
    Array<TPixel, 2> dst({dst_height, dst_width}, 0);
    for(size_t i = 0;i < dst_height;i++){
        for(size_t j = 0;j < src_width;j++){
            //TODO: CONV OPERATION FOR Array<*, 2>
        }
    }
}

#endif //IMAGEPROCESSOR6_MATH_H
