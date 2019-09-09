#include <QApplication>
#include <QTextStream>
#include <QFileSystemModel>
#include <QTreeView>
#include "application.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    auto* window = new ImageWindow();
    window->addPackage(Packages::basePackage(window));
    window->show();
    window->properties().setProperty("foreground", "#000000");
    window->properties().setProperty("background", "#FFFFFF");
    window->properties().setProperty("ocrLang", "eng");
    window->properties().setProperty("srcLang", "en");
    window->properties().setProperty("dstLang", "zh-cn");
    window->properties().setProperty("font", "monospace");
    window->properties().setProperty("fontSize", "18");
    QApplication::exec();
}