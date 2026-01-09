#pragma once

// // CRTP Singleton
// template <typename T>
// class Singleton
// {
// protected:
//     Singleton()  = default;
//     ~Singleton() = default;

//     Singleton(const Singleton&)            = delete;
//     Singleton& operator=(const Singleton&) = delete;

// public:
//     static T& instance()
//     {
//         static T inst;
//         return inst;
//     }

//     static T* getInstance() { return &instance(); }
// };

#define DECLARE_SINGLETON(ClassName)                  \
private:                                              \
    ClassName()                            = default; \
    ~ClassName()                           = default; \
    ClassName(const ClassName&)            = delete;  \
    ClassName& operator=(const ClassName&) = delete;  \
    ClassName(ClassName&&)                 = delete;  \
    ClassName& operator=(ClassName&&)      = delete;  \
                                                      \
public:                                               \
    static ClassName& instance()                      \
    {                                                 \
        static ClassName inst;                        \
        return inst;                                  \
    }                                                 \
    static ClassName* getInstance()                   \
    {                                                 \
        return &instance();                           \
    }