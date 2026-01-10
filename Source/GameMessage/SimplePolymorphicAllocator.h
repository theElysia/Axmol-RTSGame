#pragma once

#include "axmol.h"
#include <cstdlib>
#include <memory>
#include <new>

namespace simple_pmr
{

// 简单的单调缓冲区资源（不实际回收内存，只能整体释放）
class monotonic_buffer_resource
{
private:
    void* _buffer;
    size_t _size;
    size_t _offset;

public:
    monotonic_buffer_resource(void* buffer, size_t size) : _buffer(buffer), _size(size), _offset(0) {}

    void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t))
    {
        // 简单对齐计算
        size_t aligned_offset = (_offset + alignment - 1) & ~(alignment - 1);

        if (aligned_offset + bytes > _size)
        {
            throw std::bad_alloc();
        }

        void* ptr = static_cast<uint8_t*>(_buffer) + aligned_offset;
        _offset   = aligned_offset + bytes;
        return ptr;
    }

    void deallocate(void* p, size_t bytes, size_t alignment = alignof(std::max_align_t))
    {
        // 单调缓冲区不单独释放内存
    }

    void release()
    {
        _offset = 0;  // 重置偏移量，重用内存
    }
};

// 简单的多态分配器
template <typename T>
class polymorphic_allocator
{
private:
    monotonic_buffer_resource* _resource;

public:
    using value_type = T;

    polymorphic_allocator(monotonic_buffer_resource* resource) noexcept : _resource(resource) {}

    template <typename U>
    polymorphic_allocator(const polymorphic_allocator<U>& other) noexcept : _resource(other.resource())
    {}

    T* allocate(size_t n) { return static_cast<T*>(_resource->allocate(n * sizeof(T), alignof(T))); }

    void deallocate(T* p, size_t n) { _resource->deallocate(p, n * sizeof(T), alignof(T)); }

    // 修复：正确的 new_object 方法实现
    template <typename U, typename... Args>
    U* new_object(Args&&... args)
    {
        // 关键修复：为 U 类型分配内存，而不是 T 类型
        void* memory = _resource->allocate(sizeof(U), alignof(U));
        U* ptr       = nullptr;
        try
        {
            ptr = ::new (memory) U(std::forward<Args>(args)...);
        }
        catch (...)
        {
            _resource->deallocate(memory, sizeof(U), alignof(U));
            throw;
        }
        return ptr;
    }

    monotonic_buffer_resource* resource() const { return _resource; }

    // 添加转换构造函数所需的资源访问方法
    template <typename U>
    friend class polymorphic_allocator;
};

}  // namespace simple_pmr