//
// Created by chenjs on 19-6-14.
//

#ifndef IMAGEPROCESSOR6_STREAM_H
#define IMAGEPROCESSOR6_STREAM_H

#include <cstddef>
#include <QSet>
#include "array.h"

template <typename TLabel, typename TPixel, size_t SzDim>
class Stream{
private:
    using TChannel = Array<TPixel, SzDim>;
    using TSchema = FixedArray<size_t, SzDim>;
    TSchema _schema;
    QMap<TLabel, Array<TPixel, SzDim>> _channels;
    QSet<TLabel> _labels;
public:
    explicit Stream(const TSchema& schema){
        _schema = schema;
    }
    void insert(TLabel label, const TChannel& channel){
        _labels.insert(label);
        _channels.insert(label, channel);
    }
    TChannel& create(TLabel label){
        _channels.insert(label, TChannel(_schema, 0));
        _labels.insert(label);
        return _channels[label];
    }
    const QSet<TLabel>& labels() const{
        return _labels;
    }
    TChannel& channel(TLabel label){
        return *_channels.find(label);
    }
    const TChannel& channel(TLabel label) const{
        return *_channels.find(label);
    }
    bool contains(TLabel label) const{
        return _channels.contains(label);
    }
    const TSchema& size() const{
        return _schema;
    }
    size_t size(size_t dim) const{
        return _schema[dim];
    }
};

#endif //IMAGEPROCESSOR6_STREAM_H
