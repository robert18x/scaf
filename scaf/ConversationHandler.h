#pragma once
#include <atomic>
#include <cassert>
#include <expected>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "AclMessage.h"
#include "Error.h"
#include "SynchronizedMap.h"
#include "Uid.h"
#include "utils.h"

namespace scaf {

template <typename _Agent>
class ConversationHandler {
public:
    explicit ConversationHandler(_Agent* correspondingAgent) : correspondingAgent(correspondingAgent) {}

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

private:
    std::shared_ptr<Conversation> createNewConversation(const UniqueConversationId& uid) {
        std::shared_ptr<Conversation> conversation = correspondingAgent->createBehaviour(uid);
        return activeConversations.emplace(auto{uid}, std::move(conversation));
    }

    void handleConversation(const UniqueConversationId& uid, Conversation& conversation, const AclMessage& message) {
        std::expected<void, Error> ret = utils::safeCall([&]{ return conversation.handleReceivedMessage(message); });

        if (not ret.has_value()) {      // remove conversation on error
            correspondingAgent->errorHandler.handle(ret.error());
            removeConversation(uid);
        }
        if (conversation.isFinished())  // if is finished also remove conversation
            removeConversation(uid);
    }

    constexpr decltype(AclMessage::conversationId) generateConversationId() {
        return conversationIdGenerator++;
    }

    SynchronizedMap<UniqueConversationId, std::shared_ptr<Conversation>> activeConversations;
    std::atomic<decltype(AclMessage::conversationId)> conversationIdGenerator = 0;
    _Agent* correspondingAgent;
};

}
