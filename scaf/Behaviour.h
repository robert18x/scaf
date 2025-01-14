#pragma once

#include "ConversationHandler.h"
#include "Error.h"
#include "Uid.h"

#include <expected>

namespace scaf {

template <typename _Agent>
class Behaviour {
public:
    explicit constexpr Behaviour(_Agent* agent, UniqueConversationId uid) : uid(uid), agent(agent) {}
    explicit constexpr Behaviour(const Behaviour&) = default;
    constexpr Behaviour(Behaviour&& o) : uid(std::move(o.uid)), agent(o.agent) {
        o.agent = nullptr;
    }
    constexpr virtual ~Behaviour() = default;

    constexpr std::expected<void, Error> handleReceivedMessage(const AclMessage& message) {
        nextReplyWith = message.replyWith;
        return safeCall([&](){ return handleReceivedMessageImpl(message); });
    }

    constexpr virtual bool isFinished() = 0;
    using Agent = _Agent;

    constexpr UniqueConversationId getUid() const { return uid; }

protected:

    constexpr virtual std::expected<void, Error> handleReceivedMessageImpl(const AclMessage&) = 0;

    std::expected<void, Error> sendMessage(scaf::AclMessage&& message) {
        message.inReplyTo = std::exchange(nextReplyWith, std::nullopt);
        return agent->sendMessage(*this, std::move(message));
    }

    template <typename T>
    friend class ConversationHandler;

    UniqueConversationId uid;
    std::optional<std::string> nextReplyWith;
    _Agent* agent;
};

}
