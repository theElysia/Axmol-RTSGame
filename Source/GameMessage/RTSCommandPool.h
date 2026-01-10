#pragma once

#include "omp.h"
#include "GameMessage/GameCommand.h"
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

// 条件编译：检测平台是否支持 <memory_resource>
#if defined(_WIN32) || (__cplusplus >= 201703L && __has_include(<memory_resource>))
#    include <memory_resource>
#    define USE_STD_PMR 1
namespace command_pool_pmr = std::pmr;
#else
// Android等不支持std::pmr的平台使用自定义简单实现
#    define USE_STD_PMR 0
#    include "SimplePolymorphicAllocator.h"  // 需要创建这个头文件
namespace command_pool_pmr = simple_pmr;
#endif

class GameObjectManager;

/**
 * @brief 使用内存池，双缓冲，用omp对命令并行批量处理
 * commandPool.processPreviousFrame(gameObjectManager);
 * commandPool.swapBuffers();
 * commandPool.addCommand<T>(args)
 */
class RTSCommandPool
{
private:
    // 帧缓冲区结构
    struct FrameBuffer;

    // 内存池配置
    static constexpr size_t MAX_COMMANDS_PER_FRAME = 10000;
    static constexpr size_t MAX_COMMAND_SIZE       = 64;  // 最大命令对象大小
    static constexpr size_t ALIGNMENT              = alignof(std::max_align_t);

    using CommandDeleter = std::function<void(GameCommand*)>;

public:
    RTSCommandPool() = default;

    RTSCommandPool(const RTSCommandPool&)            = delete;
    RTSCommandPool& operator=(const RTSCommandPool&) = delete;

    // 添加命令到当前帧缓冲区，并自动分组
    template <typename T, typename... Args>
    void addCommand(Args&&... args)
    {
        static_assert(std::is_base_of_v<GameCommand, T>, "T must be derived from GameCommand");

        FrameBuffer& currentBuffer = frameBuffers[currentBufferIndex];

        auto cmd = this->createCommand<T>(currentBuffer, std::forward<Args>(args)...);

        GameCommand::CommandType cmdType = cmd->getType();
        auto& commandGroup               = currentBuffer.getCommandGroup(cmdType);

        commandGroup.push_back(std::move(cmd));
        currentBuffer.totalCommands++;

        if (currentBuffer.totalCommands >= MAX_COMMANDS_PER_FRAME)
        {
            handleCommandOverflow();
        }
    }

    // 处理上一帧的命令
    // 依照优先级顺序批次处理（CommandType），优化缓存
    void processPreviousFrame(GameObjectManager* manager);

    // 交换双缓冲区（每帧结束时调用）
    void swapBuffers();

    // // 统计信息
    // struct Statistics
    // {
    //     size_t totalCommands;
    //     std::array<size_t, GameCommand::CommandType::TOTAL_NUM> commandsByType;
    //     size_t memoryUsage;
    // };

    // // 获得当前帧命令池统计信息
    // Statistics getStatistics() const;

    // 清空所有缓冲区
    void clearAll();

private:
    void processCommandGroup(GameCommand::CommandType type, FrameBuffer& buffer, GameObjectManager* manager);

    void processCommandGroupParallel(const std::vector<std::unique_ptr<GameCommand, CommandDeleter>>& commandGroup,
                                     GameObjectManager* manager);

    void executeSingleCommand(GameCommand* cmd, GameObjectManager* manager);

    void handleCommandOverflow();

private:
    // 帧缓冲区结构
    struct FrameBuffer
    {
        alignas(ALIGNMENT) std::array<uint8_t, MAX_COMMANDS_PER_FRAME * MAX_COMMAND_SIZE> memoryPool;

#if USE_STD_PMR
        // 使用标准库的pmr
        std::pmr::monotonic_buffer_resource memoryResource;
        std::pmr::polymorphic_allocator<GameCommand> allocator;
#else
        // 使用自定义的内存资源
        simple_pmr::monotonic_buffer_resource memoryResource;
        simple_pmr::polymorphic_allocator<GameCommand> allocator;
#endif

        // 按命令类型分组存储
        std::array<std::vector<std::unique_ptr<GameCommand, std::function<void(GameCommand*)>>>,
                   GameCommand::CommandType::TOTAL_NUM>
            commandGroups;

        size_t totalCommands = 0;

        FrameBuffer() : memoryResource(memoryPool.data(), memoryPool.size()), allocator(&memoryResource)
        {
            size_t avgPerGroup = MAX_COMMANDS_PER_FRAME / commandGroups.size();
            for (auto& group : commandGroups)
            {
                group.reserve(avgPerGroup);
            }
        }

        void reset()
        {
            for (auto& group : commandGroups)
            {
                group.clear();
            }
            memoryResource.release();  // 释放所有内存，重置memoryResource
            totalCommands = 0;
        }

        std::vector<std::unique_ptr<GameCommand, std::function<void(GameCommand*)>>>& getCommandGroup(
            GameCommand::CommandType type)
        {
            return commandGroups[static_cast<size_t>(type)];
        }

        size_t getGroupCount(GameCommand::CommandType type) const
        {
            return commandGroups[static_cast<size_t>(type)].size();
        }
    };

    // 双缓冲区
    FrameBuffer frameBuffers[2];
    int currentBufferIndex    = 0;  // 当前收集命令的缓冲区
    int processingBufferIndex = 1;  // 当前正在处理的缓冲区

    // 创建由memoryResource管理内存的command
    template <typename T, typename... Args>
    std::unique_ptr<GameCommand, CommandDeleter> createCommand(FrameBuffer& buffer, Args&&... args)
    {
        // 使用polymorphic_allocator直接创建对象，内存由monotonic_buffer_resource自动管理
        T* cmd = buffer.allocator.new_object<T>(std::forward<Args>(args)...);

        // 删除器只需要调用析构函数，内存由memoryResource统一管理
        CommandDeleter deleter = [&buffer](GameCommand* p) {
            if (p)
            {
                // 调用析构函数
                p->~GameCommand();
                // 内存会在FrameBuffer::reset()时通过memoryResource.release()统一释放
            }
        };

        return std::unique_ptr<GameCommand, CommandDeleter>(cmd, deleter);
    }
};