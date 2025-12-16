// 单例类，方便别的类继承
#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template <typename T>
class Singleton
{
public:
    static std::shared_ptr<T> instance()
    {
        static std::shared_ptr<T> obj{ new T() };
        return obj;
    }

    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

protected:
    Singleton() = default;
};

#endif // SINGLETON_H
