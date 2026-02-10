#pragma once
#include <memory>
//单例类
template <typename T>
class Singleton
{
protected:
    // 构造函数受保护
    // 防止外部直接 new Singleton<T>
    // 保证了只能由派生类或本模板内部构造
    Singleton() = default;

    // 虚析构函数
    // 确保通过基类指针销毁派生类对象时行为正确
    virtual ~Singleton() = default;

public:
    // 删除拷贝构造函数
    // 防止通过拷贝制造多个实例
    Singleton(const Singleton&) = delete;

    // 删除拷贝赋值运算符
    // 防止赋值语义破坏单例约束
    Singleton& operator=(const Singleton&) = delete;

    // 获取单例实例（共享指针形式）
    static std::shared_ptr<T> GetInstance()
    {
        // C++11 保证：
        // 1. static 局部变量只初始化一次
        // 2. 初始化过程线程安全
        // 3. 延迟到首次调用时创建对象
        static std::shared_ptr<T> instance{ new T };

        // 返回 shared_ptr 的副本
        // 引用计数 +1
        return instance;
    }
};
