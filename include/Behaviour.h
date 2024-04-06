#pragma once
#include "ConversationHandler.h"
#include <future>

namespace scaf {

template <typename _Agent>
class Behaviour {
public:
    explicit Behaviour(_Agent* agent) : agent(agent) {}
    explicit Behaviour(const Behaviour&) = default;
    Behaviour(Behaviour&& o) : agent(o.agent), uniqueConversationId(std::move(o.uniqueConversationId)) {
        o.agent = nullptr;
    }
    virtual ~Behaviour() = default;

    virtual void handleReceivedMessage(const AclMessage&) = 0;
    virtual std::future<Behaviour<_Agent>*> start() = 0;

    virtual bool isFinished() = 0;
    using Agent = _Agent;

protected:
    _Agent* agent;

    template <typename T>
    friend class ConversationHandler;

    void setUid(UniqueConversationId uid) {
        uniqueConversationId = uid;
    }
    UniqueConversationId uniqueConversationId;
};

}
