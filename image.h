//
// Created by chenjs on 19-6-13.
//

#ifndef IMAGEPROCESSOR6_IMAGE_H
#define IMAGEPROCESSOR6_IMAGE_H

#include <QMap>
#include <QRect>
#include "stream.h"

template <typename, size_t>
class Array;

enum class ChannelLabel{
    RED, GREEN, BLUE, GRAY
};

inline uint qHash(ChannelLabel label, uint seed = 0){
    return (uint)label ^ seed;
}

template <typename TPixel>
using ImageChannel = Array<TPixel, 2>;

template <typename TPixel>
using Image = Stream<ChannelLabel, TPixel, 2>;

typedef Image<u8> U8Image;

typedef Image<f32> F32Image;

#endif //IMAGEPROCESSOR6_IMAGE_H
