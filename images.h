//
// Created by chenjs on 19-6-14.
//

#ifndef IMAGEPROCESSOR6_IMAGES_H
#define IMAGEPROCESSOR6_IMAGES_H

#include <QImage>
//#include <opencv4/opencv2/opencv.hpp>
#include <QTemporaryFile>
#include <QDebug>
#include "image.h"

static Image<u8> qImage_to_rgb(const QImage& qImage){
    Image<u8> image({(usize)qImage.height(), (usize)qImage.width()});
    ImageChannel<u8>& redChannel = image.create(ChannelLabel::RED);
    ImageChannel<u8>& greenChannel = image.create(ChannelLabel::GREEN);
    ImageChannel<u8>& blueChannel = image.create(ChannelLabel::BLUE);
    for(size_t i = 0;i < qImage.height();i++){
        auto& redRow = redChannel[i];
        auto& greenRow = greenChannel[i];
        auto& blueRow = blueChannel[i];
        for(size_t j = 0;j < qImage.width();j++){
            QRgb pixel = qImage.pixel(j, i);
            u8 red = (u8)qRed(pixel);
            u8 green = (u8)qGreen(pixel);
            u8 blue = (u8)qBlue(pixel);
            redRow[j] = red;
            greenRow[j] = green;
            blueRow[j] = blue;
        }
    }
    return image;
}

static Image<u8> rgb_to_gray(const Image<u8>& rgbImage){
    Image<u8> grayScaleImage(rgbImage.size());
    const ImageChannel<u8>& redChannel = rgbImage.channel(ChannelLabel::RED);
    const ImageChannel<u8>& greenChannel = rgbImage.channel(ChannelLabel::GREEN);
    const ImageChannel<u8>& blueChannel = rgbImage.channel(ChannelLabel::BLUE);
    ImageChannel<u8>& grayChannel = grayScaleImage.create(ChannelLabel::GRAY);
    for(size_t i = 0;i < rgbImage.size(0);i++){
        auto& redRow = redChannel[i];
        auto& greenRow = greenChannel[i];
        auto& blueRow = blueChannel[i];
        auto& grayRow = grayChannel[i];
        for(size_t j = 0;j < rgbImage.size(1);j++){
            u8 red = redRow[j];
            u8 green = greenRow[j];
            u8 blue = blueRow[j];
            grayRow[j] = (u8)qGray(red, green, blue);
        }
    }
    return grayScaleImage;
}

static QImage rgb_to_qImage(const Image<u8>& rgbImage){
    QImage qImage(rgbImage.size(1), rgbImage.size(0), QImage::Format_RGB888);
    const ImageChannel<u8>& redChannel = rgbImage.channel(ChannelLabel::RED);
    const ImageChannel<u8>& greenChannel = rgbImage.channel(ChannelLabel::GREEN);
    const ImageChannel<u8>& blueChannel = rgbImage.channel(ChannelLabel::BLUE);
    for(size_t i = 0;i < qImage.height();i++){
        auto& redRow = redChannel[i];
        auto& greenRow = greenChannel[i];
        auto& blueRow = blueChannel[i];
        for(size_t j = 0;j < qImage.width();j++){
            u8 red = redRow[j];
            u8 green = greenRow[j];
            u8 blue = blueRow[j];
            QRgb pixel = qRgb(red, green, blue);
            qImage.setPixel(j, i, pixel);
        }
    }
    return qImage;
}

static QImage to_qImage(const Image<u8>& u8Image){
    return rgb_to_qImage(u8Image);
}

//static void cvImage_show(const cv::Mat& mat){
//    cv::imshow("imshow", mat);
//    cv::waitKey(0);
//}

//static cv::Mat qImage_to_cvImage(const QImage& qImage){
//    QTemporaryFile temp("XXXXXX.png");
//    temp.open();
//    qImage.save(&temp, "PNG");
//    return cv::imread(temp.fileName().toStdString());
//}

//static cv::Mat rgb_to_cvImage(const Image<u8>& rgb){
//    QImage qImage = rgb_to_qImage(rgb);
//    return qImage_to_cvImage(qImage);
//}

#endif //IMAGEPROCESSOR6_IMAGES_H
