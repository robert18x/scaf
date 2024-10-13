#pragma once
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <expected>
#include "Error.h"
#include "AclMessage.h"
#include "Uid.h"

namespace scaf {

template <typename _Agent>
class ConversationHandler {
public:
    explicit ConversationHandler(_Agent* correspondingAgent) : correspondingAgent(correspondingAgent) {}

private:
    using Conversation = _Agent::AgentBehaviour;

public:
    void handleMessage(const AclMessage& message) {
        UniqueConversationId uid(message.conversationId, message.sender);
        if (auto it = activeConversations.find(uid); it != activeConversations.end()) {
            handleConversation(it->first, *it->second, message);
        } else {
            Conversation& conversation = createNewConversation(uid);
            handleConversation(uid, conversation, message);
        }
    }

    std::tuple<std::reference_wrapper<Conversation>, UniqueConversationId> createNewConversation(const decltype(AclMessage::receiver)& receiver) {
        UniqueConversationId uid(generateConversationId(), receiver);
        Conversation& conversation = createNewConversation(uid);
        return std::make_pair(std::reference_wrapper(conversation), uid);
    }

private:
    Conversation& createNewConversation(const UniqueConversationId& uid) {
        std::unique_ptr<Conversation> conversation = correspondingAgent->createBehaviour(uid);
        [[maybe_unused]] auto [iterator, inserted] = activeConversations.emplace(uid, std::move(conversation));
        assert(inserted == true);
        return *iterator->second;
    }

    void handleConversation(const UniqueConversationId& uid, Conversation& conversation, const AclMessage& message) {
        std::expected<void, Error> ret;
        try {
            ret = conversation.handleReceivedMessage(message);
        } catch (const std::exception& e) {  // unhandled exception - invalid conversation
            ret = std::unexpected(Error(RetCode::generic_error, e.what()));
        } catch (...) {  // unhandled exception - invalid conversation
            ret = std::unexpected(Error(RetCode::generic_error, std::string{}));
        }
        if (not ret.has_value()) {
            correspondingAgent->errorHandler.handle(ret.error());
        }
        if (conversation.isFinished())  // if finnished also remove conversation
            removeConversation(uid);
    }

    void removeConversation(const UniqueConversationId& uid) {
        activeConversations.erase(uid);
    }

    constexpr auto generateConversationId() {
        return conversationIdGenerator++;
    }

    decltype(AclMessage::conversationId) conversationIdGenerator = 0;
    std::map<UniqueConversationId, std::unique_ptr<Conversation>> activeConversations;
    _Agent* correspondingAgent;
};

}
