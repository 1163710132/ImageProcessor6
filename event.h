//
// Created by chenjs on 19-6-13.
//

#ifndef IMAGEPROCESSOR6_EVENT_H
#define IMAGEPROCESSOR6_EVENT_H

#include <functional>
#include <QList>

template <typename TArg>
class Event{
private:
    using pair = QPair<int, std::function<void(TArg)>>;
    QList<pair> _handlers;
    int _id = 0;
public:
    explicit Event() = default;

    int operator+=(std::function<void(TArg)> handler){
        _handlers.push_back(qMakePair(++_id, handler));
        return _id;
    }

    void operator-=(int id){
        using list = QList<pair>;
        using iterator = typename list::iterator;
        for(iterator& iter = _handlers.begin();iter != _handlers.end();++iter){
            if(iter->first == id){
                _handlers.erase(iter);
            }
        }
    }

    void operator()(TArg arg) const{
        for(const pair& handler: _handlers){
            handler.second(arg);
        }
    }
};

#endif //IMAGEPROCESSOR6_EVENT_H
