//
// Created by chenjs on 19-6-13.
//

#ifndef IMAGEPROCESSOR6_APPLICATION_H
#define IMAGEPROCESSOR6_APPLICATION_H

#include <QMainWindow>
#include <QApplication>
#include <QPainter>
#include <QScrollArea>
#include <QToolBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QPdfium>
#include <QMouseEvent>
#include <QActionGroup>
#include <QPushButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QStack>
#include <QSettings>
#include <QLabel>
#include <QClipboard>
#include "image.h"
#include "images.h"
#include "tesseract.h"
#include "translate.h"
#include "event.h"
#include "config.h"

class Files{
public:
    static bool isImage(const QString& image){
        return image.endsWith(".jpg")
        || image.endsWith(".png")
        || image.endsWith(".gif")
        || image.endsWith(".bmp");
    }
};

using PropertyListener = consumer<const QString&>;

class Property{
private:
    QString _name;
    QString _value;
    QVector<PropertyListener> _listeners;
public:
    explicit Property(QString name, QString value = QString())
    : _name(std::move(name)), _value(std::move(value)){}

    QString name(){
        return _name;
    }

    QString value(){
        return _value;
    }

    void setValue(QString value){
        if(value != _value){
            notifyValue(std::move(value));
        }
    }

    void notifyValue(QString value){
        _value = std::move(value);
        for(const auto& listener: _listeners){
            listener(_value);
        }
    }

    void addListener(PropertyListener listener){
        _listeners.push_back(std::move(listener));
    }
};

class Properties{
private:
    QMap<QString, Property*> _properties;
public:
    QString property(const QString& key){
        if(!_properties.contains(key)){
            _properties[key] = new Property(key);
        }
        return _properties[key]->value();
    }
    double numberProperty(const QString& key){
        QString value = property(key);
        return value.toDouble();
    }

    void setProperty(const QString& key, QString value){
        if(!_properties.contains(key)){
            _properties[key] = new Property(key);
        }
        _properties[key]->setValue(std::move(value));
    }

    void notifyProperty(const QString& key, QString value){
        if(!_properties.contains(key)){
            _properties[key] = new Property(key);
        }
        _properties[key]->notifyValue(std::move(value));
    }

    void setNumberProperty(const QString& key, double value){
        setProperty(key, QString::number(value));
    }

    void addListener(const QString& key, PropertyListener listener){
        if(!_properties.contains(key)){
            _properties[key] = new Property(key);
        }
        _properties[key]->addListener(std::move(listener));
    }
};

enum class MouseEventType{
    MouseRelease, MousePress, MouseMove,
};

struct MouseEvent{
    MouseEventType type = MouseEventType::MouseMove;
    QPoint pos = QPoint(0, 0);

    static MouseEvent release(QPoint pos){
        MouseEvent event;
        event.type = MouseEventType::MouseRelease;
        event.pos = pos;
        return event;
    }

    static MouseEvent press(QPoint pos){
        MouseEvent event;
        event.type = MouseEventType::MousePress;
        event.pos = pos;
        return event;
    }

    static MouseEvent move(QPoint pos){
        MouseEvent event;
        event.type = MouseEventType::MouseMove;
        event.pos = pos;
        return event;
    }
};

typedef fn<QImage(const QImage&)> ImageFilter;

class ImageSource{
private:
public:
    virtual QImage at(size_t index) = 0;
    virtual size_t size() = 0;
    virtual ~ImageSource() = trivial;
};

class PdfImageSource: public ImageSource{
private:
    QPdfium* _pdfium;
public:
    explicit PdfImageSource(QString name){
        _pdfium = new QPdfium(std::move(name));
    }
    QImage at(size_t index) override{
        return _pdfium->page(index).image();
    }
    size_t size() override{
        return _pdfium->pageCount();
    }
    ~PdfImageSource() override{
        delete _pdfium;
    }
};

class FolderImageSource: public ImageSource{
private:
    QList<QString> _fileNames;
    QString _folderName;
public:
    explicit FolderImageSource(QString folderName){
        QDir folder(folderName);
        for(auto& fileInfo: folder.entryInfoList()){
            if(Files::isImage(fileInfo.fileName())){
                _fileNames.push_back(fileInfo.filePath());
            }
        }
        _folderName = std::move(folderName);
    }
    QImage at(size_t index) override {
        return QImage(_fileNames[index]);
    }
    size_t size() override {
        return _fileNames.size();
    }
};

class SingleImageSource: public ImageSource{
private:
    QImage _image;
public:
    explicit SingleImageSource(const QString& name):_image(name){}
    explicit SingleImageSource(QImage image):_image(std::move(image)){}
    QImage at(size_t index) override{
        if(index == 0){
            return _image;
        }else{
            return QImage{};
        }
    }
    size_t size() override{
        return 1;
    }
};

class EmptyImageSource: public ImageSource{
private:
public:
    QImage at(size_t index) override {
        return QImage();
    }
    size_t size() override {
        return 1;
    }
};

class ImageSourceProvider{
private:
public:
    explicit ImageSourceProvider() = trivial;
    virtual bool support(const QString& path) = 0;
    virtual ImageSource* provide(const QString& path) = 0;
    virtual ~ImageSourceProvider() = trivial;
};

class PdfProvider: public ImageSourceProvider{
public:
    bool support(const QString& path) override {
        return path.endsWith(".pdf");
    }

    ImageSource* provide(const QString& path) override {
        return new PdfImageSource(path);
    }
};

class SingleImageProvider: public ImageSourceProvider{
public:
    bool support(const QString& path) override {
        return path.endsWith(".jpg")
        || path.endsWith(".png")
        || path.endsWith(".gif")
        || path.endsWith(".bmp");
    }

    ImageSource* provide(const QString& path) override {
        return new SingleImageSource(path);
    }
};

class FolderProvider: public ImageSourceProvider{
public:
    bool support(const QString& path) override {
        QDir dir(path);
        return dir.exists() && dir.isReadable();
    }

    ImageSource* provide(const QString& path) override {
        return new FolderImageSource(path);
    }
};

class Brush{
private:
    QString _name;
public:
    explicit Brush(QString name): _name(std::move(name)){}
    QString name(){
        return _name;
    }
    virtual bool accept(QList<MouseEvent>& track) = 0;
    virtual ImageFilter* apply(QList<MouseEvent>& track) = 0;
    virtual void paint(QList<MouseEvent>& track, QPainter& painter) = 0;
    virtual ~Brush() = trivial;

    static size_t countPress(const QList<MouseEvent>& track){
        size_t count = 0;
        for(auto& mouseEvent: track){
            if(mouseEvent.type == MouseEventType::MousePress){
                ++count;
            }
        }
        return count;
    }

    static size_t countRelease(const QList<MouseEvent>& track){
        size_t count = 0;
        for(auto& mouseEvent: track){
            if(mouseEvent.type == MouseEventType::MouseRelease){
                ++count;
            }
        }
        return count;
    }

    static size_t countMove(const QList<MouseEvent>& track){
        size_t count = 0;
        for(auto& mouseEvent: track){
            if(mouseEvent.type == MouseEventType::MouseMove){
                ++count;
            }
        }
        return count;
    }

    static QPoint press(const QList<MouseEvent>& track, size_t index){
        for(auto& mouseEvent: track){
            if(mouseEvent.type == MouseEventType::MousePress){
                if(index == 0){
                    return mouseEvent.pos;
                }else{
                    --index;
                }
            }
        }
        return QPoint(0, 0);
    }

    static QPoint release(const QList<MouseEvent>& track, size_t index){
        for(auto& mouseEvent: track){
            if(mouseEvent.type == MouseEventType::MouseRelease){
                if(index == 0){
                    return mouseEvent.pos;
                }else{
                    --index;
                }
            }
        }
        return QPoint(0, 0);
    }

    static QPoint move(const QList<MouseEvent>& track, size_t index){
        for(auto& mouseEvent: track){
            if(mouseEvent.type == MouseEventType::MouseMove){
                if(index == 0){
                    return mouseEvent.pos;
                }else{
                    --index;
                }
            }
        }
        return QPoint(0, 0);
    }
};

class Package{
private:
    QList<QPair<QString, QWidget*>> _tools;
    QList<Brush*> _brushes;
    QList<QPair<QString, QAction*>> _actions;
    QList<ImageSourceProvider*> _providers;
public:
    QList<QPair<QString, QWidget*>> tools(){
        auto tools = _tools;
        _tools.clear();
        return tools;
    }
    QList<Brush*> brushes(){
        auto brushes = _brushes;
        _brushes.clear();
        return brushes;
    }
    QList<QPair<QString, QAction*>> actions(){
        auto actions = _actions;
        _actions.clear();
        return actions;
    }
    QList<ImageSourceProvider*> providers(){
        auto providers = _providers;
        _providers.clear();
        return providers;
    }
    void addTool(const QString& category, QWidget* tool){
        _tools.push_back({category, tool});
    }
    void addBrush(Brush* brush){
        _brushes.push_back(brush);
    }
    void addAction(const QString& category, QAction* action){
        _actions.push_back({category, action});
    }
    void addProvider(ImageSourceProvider* provider){
        _providers.push_back(provider);
    }
    ~Package(){
        for(const auto& tool: _tools){
            delete tool.second;
        }
        for(auto brush: _brushes){
            delete brush;
        }
        for(const auto& action: _actions){
            delete action.second;
        }
    }
};

class ImageWidget: public QWidget{
    Q_OBJECT
private:
    QImage _image;
    Brush* _brush;
    QList<MouseEvent> _track;
    QStack<QImage> _redoes;
    QStack<QImage> _undoes;
    size_t _stackLimit;
    f64 _scale;
public:
    explicit ImageWidget(QWidget* parent = nullptr): QWidget(parent){
        _image = QImage();
        _brush = nullptr;
        setMouseTracking(true);
        _stackLimit = 10;
        _scale = 1.0;
    }
    QImage& image(){
        return _image;
    }
    void setScale(f64 scale){
        _scale = scale;
        setFixedSize(_image.size() * _scale);
        update();
    }
    void setImage(QImage image){
        _redoes.clear();
        _undoes.push_back(std::move(_image));
        _image = std::move(image);
        while(_undoes.size() > _stackLimit){
            _undoes.pop_front();
        }
        setFixedSize(_image.size() * _scale);
        update();
    }
    void applyFilter(const ImageFilter& filter){
        setImage(filter(_image));
    }
    size_t stackLimit(){
        return _stackLimit;
    }
    void setStackLimit(size_t stackLimit){
        _stackLimit = stackLimit;
    }
    bool undo(){
        if(!_undoes.empty()){
            _redoes.push_back(std::move(_image));
            _image = _undoes.takeLast();
            setFixedSize(_image.size() * _scale);
            update();
            return true;
        }else{
            return false;
        }
    }
    bool redo(){
        if(!_redoes.empty()){
            _undoes.push_back(std::move(_image));
            _image = _redoes.takeLast();
            setFixedSize(_image.size() * _scale);
            update();
            return true;
        }else{
            return false;
        }
    }
    void setBrush(Brush* brush){
        _brush = brush;
        _track.clear();
    }
    void notifyBrush(){
        if(_brush != nullptr && _brush->accept(_track)){
            ImageFilter* filter = _brush->apply(_track);
            if(filter != nullptr){
                setImage((*filter)(_image));
                delete filter;
            }
            _track.clear();
        }
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.scale(_scale, _scale);
        painter.drawImage(QPoint(0, 0), _image);
        if(_brush != nullptr){
            _brush->paint(_track, painter);
        }
    }
    void mousePressEvent(QMouseEvent* event) override {
        if(_brush != nullptr){
            _track.push_back(MouseEvent::press(event->pos() / _scale));
            notifyBrush();
            repaint();
        }
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if(_brush != nullptr){
            _track.push_back(MouseEvent::release(event->pos() / _scale));
            notifyBrush();
            repaint();
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if(_brush != nullptr){
            _track.push_back(MouseEvent::move(event->pos() / _scale));
            notifyBrush();
            repaint();
        }
    }
};

class ImageWindow: public QMainWindow{
    Q_OBJECT
private:
    ImageWidget* _imageWidget;
    QMap<QString, QToolBar*> _toolBars;
    QToolBar* _brushesToolBar;
    QActionGroup* _brushesGroup;
    QScrollArea* _scrollArea;
    QMap<QString, QString> _temp;
    QMap<QString, QPair<Brush*, QAction*>> _brushes;
    QMap<QString, QAction*> _brushActions;
    QList<ImageSourceProvider*> _providers;
    ImageSource* _imageSource;
//    size_t _index;
    Properties _properties;
public:
    explicit ImageWindow(){
        _imageWidget = new ImageWidget();
        _brushesToolBar = addToolBar("Brushes");
        _brushesGroup = new QActionGroup(this);
        _scrollArea = new QScrollArea(this);
        _scrollArea->setWidget(_imageWidget);
        _scrollArea->setAlignment(Qt::AlignCenter);
        setCentralWidget(_scrollArea);
        setMouseTracking(true);
        _scrollArea->setMouseTracking(true);
        _imageSource = new EmptyImageSource();

        _properties.addListener("index", [this](const QString& value)->void{
            bool ok;
            int intValue = value.toInt(&ok);
            if(ok){
                if(_imageSource->size() > intValue && intValue >= 0){
                    _imageWidget->setImage(_imageSource->at(intValue));
                }
            }
        });
        _properties.addListener("scale", [this](const QString& value)->void{
            bool ok;
            f64 doubleValue = value.toDouble(&ok);
            if(ok){
                _imageWidget->setScale(doubleValue);
            }
        });
        _properties.addListener("brush", [this](const QString& value)->void{
            _brushes[value].second->setChecked(true);
            _imageWidget->setBrush(_brushes[value].first);
        });
        _properties.addListener("path", [this](const QString& value)->void{
            for(auto* provider: _providers){
                if(provider->support(value)){
                    ImageSource* imageSource = provider->provide(value);
                    if(imageSource != nullptr){
                        setImageSource(imageSource);
                        break;
                    }
                }
            }
        });

        _properties.setProperty("scale", "1.0");
    }
    Properties& properties(){
        return _properties;
    }
    void setImageSource(ImageSource* imageSource){
        delete _imageSource;
        _imageSource = imageSource;
        properties().setProperty("count", QString::number(_imageSource->size()));
        properties().notifyProperty("index", "0");
    }
    void addTool(const QString& category, QWidget* tool){
        getToolBar(category)->addWidget(tool);
    }
    void addBrush(Brush* brush){
        QAction* action = new QAction(brush->name());
        action->setCheckable(true);
        action->setChecked(false);
        connect(action, &QAction::triggered, [=](bool checked)->void{
            if(checked){
                _properties.setProperty("brush", brush->name());
            }
        });
        _brushesGroup->addAction(action);
        _brushes[brush->name()] = qMakePair(brush, action);
        _brushesToolBar->addAction(action);
    }
    void addAction(const QString& category, QAction* action){
        getToolBar(category)->addAction(action);
    }
    void addProvider(ImageSourceProvider* provider){
        _providers.push_back(provider);
    }
    QToolBar* getToolBar(const QString& category){
        if(!_toolBars.contains(category)){
            _toolBars.insert(category, addToolBar(category));
        }
        return _toolBars[category];
    }
    void addPackage(Package* package){
        for(const auto& tool: package->tools()){
            addTool(tool.first, tool.second);
        }
        for(auto* brush: package->brushes()){
            addBrush(brush);
        }
        for(const auto& action: package->actions()){
            addAction(action.first, action.second);
        }
        for(auto* provider: package->providers()){
            addProvider(provider);
        }
        delete package;
    }
    void setImage(QImage image){
        _imageWidget->setImage(std::move(image));
    }
    QImage& image(){
        return _imageWidget->image();
    }
    bool redo(){
        return _imageWidget->redo();
    }
    bool undo(){
        return _imageWidget->undo();
    }
    QString getOpenFile(const QString& filter = ""){
        return QFileDialog::getOpenFileName(this, "Open File", "", filter);
    }
    QString getSaveFile(const QString& filter = ""){
        return QFileDialog::getSaveFileName(this, "Save File", "", filter);
    }
    QString getFolder(){
        return QFileDialog::getExistingDirectory(this);
    }
    QString getArg(const QString& name, const QString& defaultValue = QString()){
        QString temp = defaultValue;
        if(_temp.contains(name)){
            temp = _temp[name];
        }
        temp = QInputDialog::getText(this, name, name, QLineEdit::Normal, temp);
        _temp[name] = temp;
        return temp;
    }
    void showMessage(const QString& message){
        QMessageBox::information(this, "info", message);
    }
};

class CutBrush: public Brush{
public:
    explicit CutBrush(): Brush("Cut"){}
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QPoint first = press(track, 0);
        QPoint last = release(track, 0);
        QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
        QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
        return new ImageFilter([=](const QImage& image)->QImage{
            return image.copy(QRect(tl, br));
        });
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {
        if(countPress(track) > 0){
            painter.setBrush(QBrush(QColor(255, 255, 255, 63)));
            QPoint first = press(track, 0);
            QPoint last = track.last().pos;
            QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
            QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
            painter.drawRect(QRect(tl, br));
        }
    }
};

class CursorBrush: public Brush{
public:
    explicit CursorBrush(): Brush("Cursor"){}

    bool accept(QList<MouseEvent>& track) override {
        track.clear();
        return false;
    }

    ImageFilter* apply(QList<MouseEvent>& track) override {
        return nullptr;
    }

    void paint(QList<MouseEvent>& track, QPainter& painter) override {

    }
};

class RectBrush: public Brush{
private:
    ImageWindow* _context;
public:
    explicit RectBrush(ImageWindow* context): Brush("Rect"){
        _context = context;
    }
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QString colorText = _context->getArg("color");
        if(!colorText.isEmpty()){
            QPoint first = press(track, 0);
            QPoint last = release(track, 0);
            QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
            QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
            QColor color(colorText);
            return new ImageFilter([=](const QImage& image)->QImage{
                QImage copy(image);
                for(size_t i = qMax(tl.y(), 0);i < qMin(br.y(), image.height());i++){
                    for(size_t j = qMax(tl.x(), 0);j < qMin(br.x(), image.width());j++){
                        copy.setPixelColor(j, i, color);
                    }
                }
                return copy;
            });
        }else{
            return nullptr;
        }
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {
        if(countPress(track) > 0){
            painter.setBrush(QBrush(QColor(255, 255, 255, 63)));
            QPoint first = press(track, 0);
            QPoint last = track.last().pos;
            QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
            QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
            painter.drawRect(QRect(tl, br));
        }
    }
};

class TesseractBrush: public Brush{
private:
    ImageWindow* _context;
public:
    explicit TesseractBrush(ImageWindow* context): Brush("Tesseract"){
        _context = context;
    }
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QPoint first = press(track, 0);
        QPoint last = release(track, 0);
        QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
        QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
        return new ImageFilter([=](const QImage& image)->QImage{
            QImage rectImage = image.copy(QRect(tl, br));
            _context->showMessage(tesseract(qImage_to_rgb(rectImage), _context->getArg("lang")));
            return image;
        });
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {
        if(countPress(track) > 0){
            painter.setBrush(QBrush(QColor(255, 255, 255, 63)));
            QPoint first = press(track, 0);
            QPoint last = track.last().pos;
            QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
            QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
            painter.drawRect(QRect(tl, br));
        }
    }
};

class ForegroundPickerBrush: public Brush{
private:
    ImageWindow* _context;
public:
    explicit ForegroundPickerBrush(ImageWindow* context): Brush("ForegroundPicker"){
        _context = context;
    }
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QPoint last = release(track, 0);
        QRgb pixel = _context->image().pixel(last);
        _context->properties().setProperty("foreground", QColor(pixel).name());
        return nullptr;
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {}
};

class BackgroundPickerBrush: public Brush{
private:
    ImageWindow* _context;
public:
    explicit BackgroundPickerBrush(ImageWindow* context): Brush("BackgroundPicker"){
        _context = context;
    }
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QPoint last = release(track, 0);
        QRgb pixel = _context->image().pixel(last);
        _context->properties().setProperty("background", QColor(pixel).name());
        return nullptr;
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {}
};

class TranslatorBrush: public Brush{
private:
    ImageWindow* _context;
public:
    explicit TranslatorBrush(ImageWindow* context): Brush("Translator"){
        _context = context;
    }
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QPoint first = press(track, 0);
        QPoint last = release(track, 0);
        QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
        QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
        return new ImageFilter([=](const QImage& image)->QImage{
            QImage rectImage = image.copy(QRect(tl, br));
            QString output = tesseract(qImage_to_rgb(rectImage), _context->properties().property("ocrLang"));
            QString translated;
            if(USE_CLI_TRANSLATOR){
                 translated = cli_translate(output,
                         _context->properties().property("srcLang"),
                         _context->properties().property("dstLang"));
                _context->showMessage(translated);
            }else{
                translated = google_translate_qstring(output,
                        _context->properties().property("srcLang"),
                        _context->properties().property("dstLang"));
                _context->showMessage(translated);
            }
            return image;
        });
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {
        if(countPress(track) > 0){
            painter.setBrush(QBrush(QColor(255, 255, 255, 63)));
            QPoint first = press(track, 0);
            QPoint last = track.last().pos;
            QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
            QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
            painter.drawRect(QRect(tl, br));
        }
    }
};

class TranslateAndPasteBrush: public Brush{
private:
    ImageWindow* _context;
public:
    explicit TranslateAndPasteBrush(ImageWindow* context): Brush("TranslateAndPaste"){
        _context = context;
    }
    bool accept(QList<MouseEvent>& track) override {
        return countPress(track) > 0 && countRelease(track) > 0;
    }
    ImageFilter* apply(QList<MouseEvent>& track) override {
        QPoint first = press(track, 0);
        QPoint last = release(track, 0);
        QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
        QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
        return new ImageFilter([=](const QImage& image)->QImage{
            QImage rectImage = image.copy(QRect(tl, br));
            QString output = tesseract(qImage_to_rgb(rectImage), _context->properties().property("ocrLang"));
            QString translated;
            if(USE_CLI_TRANSLATOR){
                translated = cli_translate(output,
                                           _context->properties().property("srcLang"),
                                           _context->properties().property("dstLang"));
                _context->showMessage(translated);
            }else{
                translated = google_translate_qstring(output,
                                                      _context->properties().property("srcLang"),
                                                      _context->properties().property("dstLang"));
                _context->showMessage(translated);
            }
            QImage copy(image);
            QPainter painter(&copy);
            painter.fillRect(QRect(tl, br), QColor(_context->properties().property("background")));
            painter.setBrush(QBrush(QColor(_context->properties().property("foreground"))));
            QTextOption textOption;
            textOption.setAlignment(Qt::AlignCenter);
            textOption.setWrapMode(QTextOption::WrapAnywhere);
            QFont font;
            font.setFamily(_context->properties().property("font"));
            font.setPointSize(_context->properties().property("fontSize").toInt());
            painter.setFont(font);
            painter.drawText(QRect(tl, br), translated);
            return copy;
        });
    }
    void paint(QList<MouseEvent>& track, QPainter& painter) override {
        if(countPress(track) > 0){
            painter.setBrush(QBrush(QColor(255, 255, 255, 63)));
            QPoint first = press(track, 0);
            QPoint last = track.last().pos;
            QPoint tl(qMin(first.x(), last.x()), qMin(first.y(), last.y()));
            QPoint br(qMax(first.x(), last.x()), qMax(first.y(), last.y()));
            painter.drawRect(QRect(tl, br));
        }
    }
};

class Packages{
public:
    static QAction* createAction(const QString& name, runnable runnable, const QString& shortcut = ""){
        auto* action = new QAction(name);
        QObject::connect(action, &QAction::triggered, std::move(runnable));
        if(!shortcut.isEmpty()){
            action->setShortcut(QKeySequence::fromString(shortcut));
        }
        return action;
    }

    static QAction* createAction(const QString& name, runnable runnable, const QKeySequence& keySequence){
        auto* action = new QAction(name);
        QObject::connect(action, &QAction::triggered, std::move(runnable));
        action->setShortcut(keySequence);
        return action;
    }

    static QWidget* createPropertyView(QString propertyName, ImageWindow* imageWindow){
        auto* input = new QLineEdit();
        input->setPlaceholderText(propertyName);
        QObject::connect(input, &QLineEdit::returnPressed,[=](){
            imageWindow->properties().setProperty(propertyName, input->text());
        });
        imageWindow->properties().addListener(propertyName, [=](const QString& property){
            input->setText(property);
        });
        return input;
    }

    static Package* basePackage(ImageWindow* window){
        auto* basePackage = new Package();
        basePackage->addAction("File", createAction("OpenFile", [=]()->void{
            QString path = window->getOpenFile();
            if(!path.isEmpty()){
                window->properties().setProperty("path", path);
            }
        }));
        basePackage->addAction("File", createAction("OpenFolder", [=]()->void{
            QString path = window->getFolder();
            if(!path.isEmpty()){
                window->setImageSource(new FolderImageSource(path));
            }
        }));
        basePackage->addAction("File", createAction("SaveFile", [=]()->void{
            QString path = window->getSaveFile();
            if(!path.isEmpty()){
                window->image().save(path);
            }
        }));
        basePackage->addAction("Advance", createAction("Tesseract", [=]()->void{
            QString lang = window->getArg("language", "eng");
            QString result = tesseract(qImage_to_rgb(window->image()), lang);
            window->showMessage(result);
        }));
        basePackage->addAction("Edit", createAction("Undo", [=]()->void{
            window->undo();
        }, "Ctrl+Z"));
        basePackage->addAction("Edit", createAction("Redo", [=]()->void{
            window->redo();
        }, "Ctrl+Shift+Z"));
        basePackage->addAction("Navigate", createAction("PageUp", [=]()->void{
            size_t count = (size_t)window->properties().numberProperty("count");
            size_t index = (size_t)window->properties().numberProperty("index") - 1;
            if(index >= 0 && index < count){
                window->properties().setNumberProperty("index", index);
            }
        }, "Ctrl+Left"));
        basePackage->addAction("Navigate", createAction("PageDn", [=]()->void{
            size_t count = (size_t)window->properties().numberProperty("count");
            size_t index = (size_t)window->properties().numberProperty("index") + 1;
            if(index >= 0 && index < count){
                window->properties().setNumberProperty("index", index);
            }
        }, "Ctrl+Right"));
        basePackage->addAction("Viewport", createAction("ZoomIn", [=]()->void{
            double oldScale = window->properties().numberProperty("scale");
            window->properties().setProperty("scale", QString::number(oldScale * 2));
        }, "Ctrl++"));
        basePackage->addAction("Viewport", createAction("ZoomOut", [=]()->void{
            double oldScale = window->properties().numberProperty("scale");
            window->properties().setProperty("scale", QString::number(oldScale / 2));
        }, "Ctrl+-"));
        {
            auto* temp = new QString();
            auto* label = new QLabel();
            label->setText(window->properties().property("count"));
            window->properties().addListener("count", [=](const QString& value){
                label->setText(value);
            });
            basePackage->addTool("Page", label);
            basePackage->addTool("Page", createPropertyView("index", window));
        }
        basePackage->addTool("Color", createPropertyView("foreground", window));
        basePackage->addTool("Color", createPropertyView("background", window));
        basePackage->addTool("Ocr", createPropertyView("ocrLang", window));
        basePackage->addTool("Translate", createPropertyView("srcLang", window));
        basePackage->addTool("Translate", createPropertyView("dstLang", window));
        basePackage->addTool("Font", createPropertyView("font", window));
        basePackage->addTool("Font", createPropertyView("fontSize", window));
        basePackage->addBrush(new CursorBrush());
        basePackage->addBrush(new CutBrush());
        basePackage->addBrush(new RectBrush(window));
        basePackage->addBrush(new TesseractBrush(window));
        basePackage->addBrush(new ForegroundPickerBrush(window));
        basePackage->addBrush(new BackgroundPickerBrush(window));
        basePackage->addBrush(new TranslatorBrush(window));
        basePackage->addBrush(new TranslateAndPasteBrush(window));
        basePackage->addProvider(new PdfProvider());
        basePackage->addProvider(new SingleImageProvider());
        basePackage->addProvider(new FolderProvider());
        return basePackage;
    }
};


#endif //IMAGEPROCESSOR6_APPLICATION_H
