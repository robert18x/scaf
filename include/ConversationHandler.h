#pragma once
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "AclMessage.h"

namespace scaf {

using UniqueConversationId = std::pair<decltype(AclMessage::conversationId), decltype(AclMessage::sender)>;

template <typename _Agent>
class ConversationHandler {
public:
    explicit ConversationHandler(_Agent* correspondingAgent) : correspondingAgent(correspondingAgent) {}

private:
    using Conversation = _Agent::AgentBehaviour;

public:
    void handleMessage(const AclMessage& message) {
        UniqueConversationId uid = std::make_pair(message.conversationId, message.sender);
        auto it = activeConversations.find(uid);
        if (it != activeConversations.end()) {
            handleConversation(it->first, *it->second, message);
        } else {
            Conversation& conversation = createNewConversation(uid);
            handleConversation(uid, conversation, message);
        }
    }

    std::tuple<std::reference_wrapper<Conversation>, UniqueConversationId> createNewConversation(const decltype(AclMessage::receiver)& receiver) {
        UniqueConversationId uid = std::make_pair(conversationIdGenerator++, receiver);
        Conversation& conversation = createNewConversation(uid);
        return std::make_pair(std::reference_wrapper(conversation), uid);
    }

private:
    Conversation& createNewConversation(const UniqueConversationId& uid) {
        std::unique_ptr<Conversation> conversation = correspondingAgent->createBehaviour();
        conversation->setUid(uid);
        [[maybe_unused]] auto [iterator, inserted] = activeConversations.emplace(uid, std::move(conversation));
        assert(inserted == true);
        return *iterator->second;
    }

    void handleConversation(const UniqueConversationId& uid, Conversation& conversation, const AclMessage& message) {
        try {
            conversation.handleReceivedMessage(message);
        } catch (...) {  // unhandled exception - invalid conversation
            removeConversation(uid);
        }
        if (conversation.isFinished())  // if finnished also remove conversation
            removeConversation(uid);
    }

    void removeConversation(const UniqueConversationId& uid) {
        activeConversations.erase(uid);
    }

    decltype(AclMessage::conversationId) conversationIdGenerator = 0;
    std::map<UniqueConversationId, std::unique_ptr<Conversation>> activeConversations;
    _Agent* correspondingAgent;
};

}
