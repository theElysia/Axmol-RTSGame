#pragma once

class GameMessage
{
public:
    enum class MessageType
    {
        UNDEF,

        REQUEST_ATTACK_SINGLE,
        REQUEST_ATTACK_AOE,
        REQUEST_CHASE,
        REQUEST_RANDOMMOVE,
        REQUEST_FINDENERMY,
    };

public:
    GameMessage(int senderId) : sender_id_(senderId) {}
    virtual ~GameMessage() = default;

    virtual MessageType getType() const = 0;
    int getSenderId() const { return sender_id_; }

private:
    int sender_id_ = 0;
};

class MessageRequestAttackSingle : public GameMessage
{
public:
    struct Data
    {
        int target_id;
        int damage;
    };

    MessageRequestAttackSingle(int sid, int tid, int dmg) : GameMessage(sid), data_{.target_id = tid, .damage = dmg} {}

    const Data& getData() const { return data_; }

    MessageType getType() const override { return MessageType::REQUEST_ATTACK_SINGLE; }

private:
    Data data_;
};

class MessageRequestAttackAoe : public GameMessage
{
public:
    struct Data
    {
        int target_id;
        int damage;
        float radius2;
    };

    MessageRequestAttackAoe(int sid, int tid, int dmg, float r2)
        : GameMessage(sid), data_{.target_id = tid, .damage = dmg, .radius2 = r2}
    {}

    const Data& getData() const { return data_; }

    MessageType getType() const override { return MessageType::REQUEST_ATTACK_AOE; }

private:
    Data data_;
};

class MessageRequestChase : public GameMessage
{
public:
    struct Data
    {
        int target_id;
    };
    MessageRequestChase(int sid, int tid) : GameMessage(sid), data_{.target_id = tid} {}

    const Data& getData() const { return data_; }

    MessageType getType() const override { return MessageType::REQUEST_CHASE; }

private:
    Data data_;
};

class MessageRequestFindEnermy : public GameMessage
{
public:
    struct Data
    {
        float radius2;
    };
    MessageRequestFindEnermy(int sid, float radius2) : GameMessage(sid), data_{.radius2 = radius2} {}

    const Data& getData() const { return data_; }

    MessageType getType() const override { return MessageType::REQUEST_FINDENERMY; }

private:
    Data data_;
};

class MessageRequestRandomMove : public GameMessage
{
public:
    MessageRequestRandomMove(int sid) : GameMessage(sid) {}

    MessageType getType() const override { return MessageType::REQUEST_RANDOMMOVE; }
};