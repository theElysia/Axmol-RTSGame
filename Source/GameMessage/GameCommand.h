#pragma once

#include "axmol.h"
#include <vector>

class GameCommand
{
public:
    // 与执行优先级一致
    enum CommandType
    {
        UNDEF,

        MOVE,
        GET_ATTACK,

        SELECT,
        DESELECT,
        SETTARGET,

        TOTAL_NUM
    };

public:
    GameCommand(int receiverId) : receiver_id_(receiverId) {}
    virtual ~GameCommand() = default;

    virtual CommandType getType() const = 0;
    int getReceiverId() const { return receiver_id_; }

private:
    int receiver_id_ = 0;
};

// class CommandFactory
// {
// public:
//     template <typename T, typename... Args>
//     static std::unique_ptr<GameCommand> create(Args&&... args)
//     {
//         return std::make_unique<T>(std::forward<Args>(args)...);
//     }

//     // 从网络数据创建
//     // static std::unique_ptr<GameCommand> createFromNetwork(GameCommand::CommandType type, NetworkStream& stream);
// };

class CommandSelect : public GameCommand
{
public:
    CommandSelect(int tid) : GameCommand(tid) {}
    CommandType getType() const override { return CommandType::SELECT; }
};

class CommandDeSelect : public GameCommand
{
public:
    CommandDeSelect(int tid) : GameCommand(tid) {}
    CommandType getType() const override { return CommandType::DESELECT; }
};

class CommandSetTarget : public GameCommand
{
public:
    struct Data
    {
        int new_target;
    };

    CommandSetTarget(int tid, int newTarget) : GameCommand(tid), data_{.new_target = newTarget} {}

    const Data& getData() const { return data_; }

    CommandType getType() const override { return CommandType::SETTARGET; }

private:
    Data data_;
};

class CommandMove : public GameCommand
{
public:
    struct Data
    {
        std::vector<ax::Vec2> path;
        int strategy;  // 优先级 0：强制移动  1：索敌  2：指定移动  3：随机移动
    };

    CommandMove(int tid, std::vector<ax::Vec2> path, int strategy = 1)
        : GameCommand(tid), data_{.path = path, .strategy = strategy}
    {
        AXLOGD("Command make {} {} {} {}", tid, data_.path[0].x, data_.path[0].y, data_.strategy);
    }

    const Data& getData() const { return data_; }

    CommandType getType() const override { return CommandType::MOVE; }

private:
    Data data_;
};

class CommandGetAttack : public GameCommand
{
public:
    struct Data
    {
        int source_id;
        int damage;
    };

    CommandGetAttack(int tid, int sid, int dmg) : GameCommand(tid), data_{.source_id = sid, .damage = dmg} {}

    const Data& getData() const { return data_; }

    CommandType getType() const override { return CommandType::GET_ATTACK; }

private:
    Data data_;
};
