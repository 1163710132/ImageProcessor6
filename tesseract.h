//
// Created by chenjs on 19-6-14.
//

#ifndef IMAGEPROCESSOR6_TESSERACT_H
#define IMAGEPROCESSOR6_TESSERACT_H

#include <QProcess>
#include <QTemporaryFile>
#include <QDebug>
#include "image.h"
#include "images.h"
#include "config.h"

static QString tesseract(const Image<u8>& image, const QString& lang = "eng"){
    QImage qImage = rgb_to_qImage(image);
    QProcess tesseract;
    QTemporaryFile inputTemp("XXXXXX.png");
    QTemporaryFile outputTemp("XXXXXX.txt");
    inputTemp.open();
    outputTemp.open();
    qImage.save(&inputTemp, "PNG");
    tesseract.setProgram(TESSERACT_PATH);
    tesseract.setArguments({inputTemp.fileName(), outputTemp.fileName().left(outputTemp.fileName().size() - 4), "-l", lang});
    tesseract.start();
    tesseract.waitForFinished();
    return QString(outputTemp.readAll());
}

#endif //IMAGEPROCESSOR6_TESSERACT_H
