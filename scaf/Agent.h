#pragma once
#include <cassert>
#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

#include "AclMessage.h"
#include "Behaviour.h"
#include "CommunicationHandler.h"
#include "ConversationHandler.h"
#include "ErrorHandler.h"
#include "JsonSerializer.h"
#include "Uid.h"

namespace scaf {  // Smart Contracting Agents Framework

template <typename _Behaviour, typename _CommunicationHandler, typename _ErrorHandler>
    requires std::derived_from<_CommunicationHandler, CommunicationHandler> and
             std::derived_from<_ErrorHandler, ErrorHandler>

class Agent {
public:
    explicit Agent(const std::string& name) : name(name), conversationHandler(this) {}
    virtual ~Agent() = default;
    Agent(const Agent&) = delete;
    Agent(Agent&&) = delete;

    void handleData(std::span<char> data) {
        try {
            std::expected message = serializer.deserialize(data);
            if (message.has_value())
                conversationHandler.handleMessage(message.value());
            else
                errorHandler.handle(message.error());
        } catch (const std::exception& e) {
            std::cerr << "Unrecognized exception in Agent::handleData member function:" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error in Agent::handleData member function!" << std::endl;
        }
    }

    using AgentBehaviour = _Behaviour;

protected:
    virtual decltype(std::declval<_Behaviour>().start()) startConversation(const decltype(AclMessage::receiver)& receiver) {
        auto [conversation, uid] = conversationHandler.createNewConversation(receiver);
        return conversation.get().start();
    }

    virtual void sendMessage(AclMessage&& message, const _Behaviour& behaviour) {
        sendMessage(std::move(message), behaviour.getUid());
    }

    friend class ConversationHandler<Agent>;
    using Super = Agent<_Behaviour, _CommunicationHandler, _ErrorHandler>;

    const std::string name;
    JsonSerializer serializer;
    _CommunicationHandler communicationHandler;
    _ErrorHandler errorHandler;
    ConversationHandler<Agent> conversationHandler;

private:
    // it is recommended to use sendMessage member function over direct communicationHandler call
    virtual void sendMessage(AclMessage&& message, UniqueConversationId uid) {
        message.sender = name;
        message.receiver = uid.sender;
        message.conversationId = uid.conversationId;
        std::expected data = serializer.serialize(message);
        if (data.has_value()) {
            communicationHandler.send(data.value());
        } else {
            errorHandler.handle(data.error());
        }
    }

    virtual std::unique_ptr<_Behaviour> createBehaviour(UniqueConversationId uid) {
        static_assert(std::derived_from<typename _Behaviour::Agent, Agent>);
        if constexpr (std::is_same_v<typename _Behaviour::Agent, std::remove_cvref_t<decltype(*this)>>) {
            return std::make_unique<_Behaviour>(this, uid);
        } else {
            auto* agentSpecialization = dynamic_cast<typename _Behaviour::Agent*>(this);
            assert(agentSpecialization != nullptr);
            return std::make_unique<_Behaviour>(agentSpecialization, uid);
        }
    }
};

}
