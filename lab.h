//
// Created by chenjs on 19-6-17.
//

#ifndef IMAGEPROCESSOR6_LAB_H
#define IMAGEPROCESSOR6_LAB_H

#include <QString>
#include <QMap>
#include <QSet>
#include <QSharedPointer>
#include <QQueue>
#include <functional>

class Resource{
private:
public:
    virtual ~Resource() = default;

    template <typename T>
    T& as(){
        return *static_cast<T*>(this);
    }
};

template <typename T>
class ResourceAdapter: public Resource{
    T* _value;
public:
    explicit ResourceAdapter(T* value){
        _value = value;
    }
    T& get(){
        return *_value;
    }
    ~ResourceAdapter() override{
        delete _value;
    }
};

typedef QSharedPointer<Resource> Handle;

class Producer{
private:
    QSet<QString> _dependencies;
public:
    explicit Producer(QSet<QString> dependencies)
    : _dependencies(std::move(dependencies)){}
    virtual Handle produce(const QMap<QString, Handle>& resources) = 0;
    virtual ~Producer()= default;

    friend class Graph;
};

class ProducerAdapter: public Producer{
private:
    using lambda = std::function<Handle(const QMap<QString, Handle>& resources)>;
    lambda _lambda;
public:
    explicit ProducerAdapter(QSet<QString> dependencies, lambda lambda)
    :Producer(std::move(dependencies)) , _lambda(std::move(lambda)){}

    Handle produce(const QMap<QString, Handle>& resources) override {
        return _lambda(resources);
    }

    ~ProducerAdapter() override = default;
};

class Graph{
private:
    QMap<QString, Producer*> _producers;
    QMap<QString, Handle> _cache;
public:
    using ResourceMap = QMap<QString, Handle>;
    QMap<QString, Handle> collect(const QSet<QString>& manifest){
        QMap<QString, Handle> collected;
        for(auto& item: manifest){
            collected.insert(item, _cache[item]);
        }
        return collected;
    }
    Handle pull(const QString& nodeName, bool cover = false){
        Producer& producer = *_producers[nodeName];
        if((!_cache.contains(nodeName)) || cover){
            for(auto& dependence: producer._dependencies){
                pull(dependence, cover);
            }
            ResourceMap args = collect(producer._dependencies);
            if(args.size() == producer._dependencies.size()){
                _cache[nodeName] = producer.produce(args);
            }
        }
        return _cache.contains(nodeName) ? _cache[nodeName] : Handle();
    }
    void push(const QString& nodeName, bool cover = false){
        Producer& producer = *_producers[nodeName];
        ResourceMap args = collect(producer._dependencies);
        if(args.size() == producer._dependencies.size()){
            push(nodeName, producer.produce(args), cover);
        }
    }
    void push(const QString& nodeName, Handle handle, bool cover = false){
        _cache[nodeName] = std::move(handle);
        for(auto iter = _producers.keyValueBegin();iter != _producers.keyValueEnd();++iter){
            Producer& producer = *iter.operator*().second;
            if(producer._dependencies.contains(nodeName)){
                if((!_cache.contains(iter.operator*().first)) || cover){
                    ResourceMap args = collect(producer._dependencies);
                    if(args.size() == producer._dependencies.size()){
                        push(iter.operator*().first, producer.produce(args), cover);
                    }
                }
            }
        }
    }
    void push(const QString& nodeName, Resource* resource){
        push(nodeName, Handle(resource));
    }
    Handle operator[](const QString& nodeName){
        return _cache.contains(nodeName) ? _cache[nodeName] : Handle();
    }
    void setProducer(const QString& nodeName, Producer* producer, bool validateCache = false){
        Producer* old = _producers.take(nodeName);
        delete old;
        if(producer != nullptr){
            _producers[nodeName] = producer;
        }
        if(validateCache){
            _cache.remove(nodeName);
        }
    }
    void setProducer(
            const QString& nodeName,
            QSet<QString> dependencies,
            std::function<Handle(const ResourceMap&)> function,
            bool validateCache = false){
        setProducer(nodeName,
                new ProducerAdapter(std::move(dependencies), std::move(function)), validateCache);
    }
    void remove(const QString& nodeName){
        _cache.remove(nodeName);
    }
};

#endif //IMAGEPROCESSOR6_LAB_H
