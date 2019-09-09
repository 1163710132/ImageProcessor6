//
// Created by chenjs on 19-6-13.
//

#ifndef IMAGEPROCESSOR6_MATRIX_H
#define IMAGEPROCESSOR6_MATRIX_H

#include <cstddef>

template <typename TPixel>
class Matrix{
private:
    TPixel* _data;
    size_t _height;
    size_t _width;
public:
    Matrix(size_t height, size_t width, TPixel defaultPixel = 0){
        _data = new TPixel[height * width];
        _height = height;
        _width = width;
        for(size_t i = 0;i < _height;i++){
            for(size_t j = 0;j < _width;j++){
                at(i, j) = defaultPixel;
            }
        }
    }
    TPixel& at(int y, int x){
        return _data[_width * y + x];
    }
    TPixel at(int y, int x) const{
        return _data[_width * y + x];
    }
    size_t height() const{
        return _height;
    }
    size_t width() const{
        return _width;
    }
    ~Matrix(){
        delete _data;
    }
};

#endif //IMAGEPROCESSOR6_MATRIX_H
