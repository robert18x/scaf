#pragma once

#include "AclMessage.h"
#include "Error.h"
#include "SynchronizedMap.h"
#include "Uid.h"
#include "utils.h"

#include <atomic>
#include <cassert>
#include <expected>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <utility>

namespace scaf {

template <typename _Agent>
class ConversationHandler {
public:
    explicit ConversationHandler(_Agent* correspondingAgent) : correspondingAgent(correspondingAgent) {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<> distrib(0, std::numeric_limits<std::uint32_t>::max);
        conversationIdGenerator = distrib(gen);
    }

private:
    using Conversation = _Agent::AgentBehaviour;
    friend _Agent;

public:
    void handleMessage(const AclMessage& message) {
        UniqueConversationId uid(message.conversationId, message.sender);
        if (auto conversation = activeConversations.get(uid)) {
            handleConversation(uid, **conversation, message);
        } else {
            std::shared_ptr<Conversation> newConversation = createNewConversation(uid);
            handleConversation(uid, *newConversation, message);
        }
    }

    std::shared_ptr<Conversation> createNewConversation(const decltype(AclMessage::receiver)& receiver) {
        UniqueConversationId uid(generateConversationId(), receiver);
        return createNewConversation(uid);
    }

    void removeConversation(const UniqueConversationId& uid) {
        activeConversations.erase(uid);
    }

    std::shared_ptr<Conversation> getConversation(const UniqueConversationId& uid) {
        return activeConversations.get(uid).value_or(nullptr);
    }

private:
    std::shared_ptr<Conversation> createNewConversation(const UniqueConversationId& uid) {
        std::shared_ptr<Conversation> conversation = correspondingAgent->createBehaviour(uid);
        return activeConversations.emplace(auto{uid}, std::move(conversation));
    }

    void handleConversation(const UniqueConversationId& uid, Conversation& conversation, const AclMessage& message) {
        std::expected<void, Error> ret = safeCall([&]{ return conversation.handleReceivedMessage(message); });

        if (not ret.has_value()) {      // remove conversation on error
            correspondingAgent->errorHandler.handle(ret.error());
            removeConversation(uid);
        }
        if (conversation.isFinished())  // if is finished, also remove conversation
            removeConversation(uid);
    }

    constexpr decltype(AclMessage::conversationId) generateConversationId() volatile {
        return conversationIdGenerator++;
    }

    SynchronizedMap<UniqueConversationId, std::shared_ptr<Conversation>> activeConversations;
    std::atomic<decltype(AclMessage::conversationId)> conversationIdGenerator;
    _Agent* correspondingAgent;
};

}
