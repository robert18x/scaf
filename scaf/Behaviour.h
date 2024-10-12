#pragma once
#include "ConversationHandler.h"
#include <future>
#include <expected>
#include "Uid.h"
#include "Error.h"

namespace scaf {

template <typename _Agent>
class Behaviour {
public:
    explicit constexpr Behaviour(_Agent* agent, UniqueConversationId uid) : agent(agent), uid(uid) {}
    explicit constexpr Behaviour(const Behaviour&) = default;
    constexpr Behaviour(Behaviour&& o) : agent(o.agent), uid(std::move(o.uid)) {
        o.agent = nullptr;
    }
    constexpr virtual ~Behaviour() = default;

    constexpr virtual std::expected<void, Error> handleReceivedMessage(const AclMessage&) = 0;
    constexpr virtual std::future<Behaviour<_Agent>*> start() = 0;

    constexpr virtual bool isFinished() = 0;
    using Agent = _Agent;

    constexpr UniqueConversationId getUid() const { return uid; }

protected:
    _Agent* agent;

    template <typename T>
    friend class ConversationHandler;

    UniqueConversationId uid;
};

}
