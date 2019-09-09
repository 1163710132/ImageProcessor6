//
// Created by chenjs on 19-6-29.
//

#ifndef IMAGEPROCESSOR6_TRANSLATE_H
#define IMAGEPROCESSOR6_TRANSLATE_H

#include <QProcess>
#include <QTemporaryFile>
#include <QDebug>
#include <string>
#include <QString>
#include "utils.h"

std::string google_translate_stdstring(std::string raw, std::string from, std::string to);

static QString google_translate_qstring(QString raw, QString from, QString to){
    std::string result = google_translate_stdstring(raw.toStdString(), from.toStdString(), to.toStdString());
    return QString::fromStdString(result);
}

static QString cli_translate(const QString& raw, const QString& srcLang, const QString& dstLang){
    QTemporaryFile outputTemp("XXXXXX.txt");
    outputTemp.open();
    QProcess trans;
    trans.setProgram("trans");
    trans.setArguments({"-b", raw, "-s", srcLang, "-t", dstLang});
    trans.setStandardOutputFile(outputTemp.fileName());
    trans.start();
    trans.waitForFinished();
    return QString(outputTemp.readAll());
}

#endif //IMAGEPROCESSOR6_TRANSLATE_H
