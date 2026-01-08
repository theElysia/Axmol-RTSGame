#pragma once

// CRTP Singleton
template <typename T>
class Singleton
{
protected:
    Singleton()  = default;
    ~Singleton() = default;

    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static T& instance()
    {
        static T inst;
        return inst;
    }

    static T* getInstance() { return &instance(); }
};