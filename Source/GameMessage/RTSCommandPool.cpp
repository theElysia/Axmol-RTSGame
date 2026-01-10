#include "GameMessage/RTSCommandPool.h"
#include "GameObject/GameObjectManager.h"

// 处理上一帧的命令
void RTSCommandPool::processPreviousFrame(GameObjectManager* manager)
{
    FrameBuffer& processBuffer = frameBuffers[processingBufferIndex];

    if (processBuffer.totalCommands == 0)
    {
        return;
    }

    // 按命令类型顺序处理
    for (size_t type = 0; type < processBuffer.commandGroups.size(); ++type)
    {
        GameCommand::CommandType cmdType = static_cast<GameCommand::CommandType>(type);
        if (cmdType != GameCommand::UNDEF)
        {
            processCommandGroup(cmdType, processBuffer, manager);
        }
    }
}

// 交换双缓冲区（每帧结束时调用）
void RTSCommandPool::swapBuffers()
{
    // 交换缓冲区索引
    currentBufferIndex    = 1 - currentBufferIndex;
    processingBufferIndex = 1 - processingBufferIndex;

    // 重置新当前缓冲区（准备接收新命令）
    frameBuffers[currentBufferIndex].reset();
}

// // 获得当前帧命令池统计信息
// RTSCommandPool::Statistics RTSCommandPool::getStatistics() const
// {
//     const FrameBuffer& currentBuffer = frameBuffers[currentBufferIndex];
//     Statistics stats{};

//     stats.totalCommands = currentBuffer.totalCommands;

//     for (size_t type = 0; type < currentBuffer.commandGroups.size(); ++type)
//     {
//         stats.commandsByType[type] = currentBuffer.commandGroups[type].size();
//     }

//     // 计算实际内存使用量
//     size_t memoryUsed = sizeof(memoryPool) - currentBuffer.memoryResource.remaining_storage();
//     stats.memoryUsage = memoryUsed > 0 ? memoryUsed : 0;

//     return stats;
// }

void RTSCommandPool::clearAll()
{
    for (auto& buffer : frameBuffers)
    {
        buffer.reset();
    }
}

void RTSCommandPool::processCommandGroup(GameCommand::CommandType type, FrameBuffer& buffer, GameObjectManager* manager)
{
    auto& commandGroup = buffer.getCommandGroup(type);

    if (commandGroup.empty())
    {
        return;
    }

    if (commandGroup.size() > 100)
    {
        processCommandGroupParallel(commandGroup, manager);
    }
    else
    {
        for (auto& cmd : commandGroup)
        {
            if (cmd)
            {
                executeSingleCommand(cmd.get(), manager);
            }
        }
    }
}

// 并行处理命令组
void RTSCommandPool::processCommandGroupParallel(
    const std::vector<std::unique_ptr<GameCommand, CommandDeleter>>& commandGroup,
    GameObjectManager* manager)
{
    // 使用OpenMP并行处理
    // #ifdef _OPENMP
    // #    pragma omp parallel for schedule(dynamic, 32)
    // #endif
    for (size_t i = 0; i < commandGroup.size(); ++i)
    {
        auto& cmd = commandGroup[i];
        if (cmd)
        {
            executeSingleCommand(cmd.get(), manager);
        }
    }
}

// 执行单个命令
void RTSCommandPool::executeSingleCommand(GameCommand* cmd, GameObjectManager* manager)
{
    auto obj = manager->getGameObjectBy(cmd->getReceiverId());
    if (obj)
    {
        obj->handleCommand(cmd);
    }
    else
    {
        AXLOGWARN("Command send to null GameObject: id %d, type %d", cmd->getReceiverId(), cmd->getType());
    }
}

// 处理命令溢出
void RTSCommandPool::handleCommandOverflow()
{
    static bool warned = false;
    if (!warned)
    {
        AXLOGWARN("Command pool reaching capacity: %zu/%zu commands", frameBuffers[currentBufferIndex].totalCommands,
                  MAX_COMMANDS_PER_FRAME);
        warned = true;
    }
}