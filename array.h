//
// Created by chenjs on 19-6-13.
//

#ifndef IMAGEPROCESSOR6_ARRAY_H
#define IMAGEPROCESSOR6_ARRAY_H

#include <QVector>
#include "utils.h"

template <typename T, size_t SzDim>
class Array;

template <typename T, size_t Sz>
class FixedArray{
private:
    T _data[Sz];
public:
    explicit FixedArray() = default;

    FixedArray(std::initializer_list<T> list){
        assert(list.size() == Sz);
        auto iter = list.begin();
        for(size_t i = 0;i < Sz;i++){
            assert(iter != list.end());
            _data[i] = *iter;
            ++iter;
        }
    }
    inline constexpr size_t size() const{
        return Sz;
    }
    inline T& operator[](size_t index){
        assert(index >= 0 && index < Sz);
        return _data[index];
    }
    inline const T& operator[](size_t index) const{
        assert(index >= 0 && index < Sz);
        return _data[index];
    }
    inline T& at(size_t index){
        assert(index >= 0 && index < Sz);
        return _data[index];
    }
    inline const T& at(size_t index) const{
        assert(index >= 0 && index < Sz);
        return _data[index];
    }
    inline T* begin(){
        return _data;
    }
    inline T* end(){
        return (T*)_data + Sz;
    }
    inline const T* begin() const{
        return _data;
    }
    inline const T* end() const{
        return (T*)_data + Sz;
    }
};

template <typename T>
class Array<T, 1>{
private:
    QVector<T> _data;
public:
    explicit Array() = default;

    explicit Array(size_t size, T defaultValue)
    : _data(size, defaultValue){}

    explicit Array(const FixedArray<size_t, 1>& size, T defaultValue)
            : _data(size.at(0), defaultValue){}

    inline T& operator[](size_t index){
        return _data[index];
    }

    inline const T& operator[](size_t index) const{
        return _data[index];
    }

    inline constexpr size_t dim() const{
        return 1;
    }

    inline size_t size(size_t) const{
        return _data.size();
    }

    inline size_t size() const{
        return _data.size();
    }
};

template <typename T, size_t SzDim>
class Array{
private:
    using DataType = Array<T, SzDim - 1>;
    QVector<Array<T, SzDim - 1>> _data;
    FixedArray<size_t, SzDim> _size;
public:
    explicit Array() = default;
    explicit Array(const FixedArray<size_t, SzDim>& size, T defaultValue){
        FixedArray<size_t, SzDim - 1> dataDim;
        for(size_t i = 1;i < size.size();i++){
            dataDim[i - 1] = size.at(i);
        }
        _data = QVector(size.at(0), DataType(dataDim, defaultValue));
        _size = size;
    }
    inline DataType& operator[](size_t index){
        return _data[index];
    }
    inline const DataType& operator[](size_t index) const{
        return _data[index];
    }
    inline constexpr size_t dim() const{
        return SzDim;
    }
    inline size_t size(size_t dim) const{
        return _size[dim];
    }
};

#endif //IMAGEPROCESSOR6_ARRAY_H
