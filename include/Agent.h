#pragma once
#include <cassert>
#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

#include "AclMessage.h"
#include "CommunicationHandler.h"
#include "ConversationHandler.h"
#include "JsonSerializer.h"

namespace scaf {  // Smart Contracting Agents Framework

template <typename _Behaviour, typename _CommunicationHandler>
    requires std::derived_from<_CommunicationHandler, CommunicationHandler>
class Agent {
public:
    explicit Agent(const std::string& name) : name(name), conversationHandler(this) {}
    virtual ~Agent() = default;
    Agent(const Agent&) = delete;
    Agent(Agent&&) = delete;

    void handleData(std::span<char> data) {
        try {
            auto message = serializer.deserialize(data);
            conversationHandler.handleMessage(message);
        } catch (const SerializationError& e) {
            std::cerr << "Deserialization error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Unrecognized error in Agent::handleData member function:" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error in Agent::handleData member function!" << std::endl;
        }
    }

    using AgentBehaviour = _Behaviour;

protected:
    virtual void startConversation(const decltype(AclMessage::receiver)& receiver) {
        auto [conversation, uid] = conversationHandler.createNewConversation(receiver);
        conversation.get().start();
    }

    // it is recommended to use sendMessage member function over direct communicationHandler call
    virtual void sendMessage(AclMessage&& message, UniqueConversationId uid) {
        message.sender = name;
        message.receiver = uid.second;
        message.conversationId = uid.first;
        std::string data = serializer.serialize(message);

        communicationHandler.send(data);
    }

    friend class ConversationHandler<Agent>;

    const std::string name;
    JsonSerializer serializer;
    _CommunicationHandler communicationHandler;
    ConversationHandler<Agent> conversationHandler;

private:
    virtual std::unique_ptr<_Behaviour> createBehaviour() {
        static_assert(std::derived_from<typename _Behaviour::Agent, Agent>);
        if constexpr (std::is_same_v<typename _Behaviour::Agent, std::remove_cvref_t<decltype(*this)>>) {
            return std::make_unique<_Behaviour>(this);
        } else {
            auto* agentSpecialization = dynamic_cast<typename _Behaviour::Agent*>(this);
            assert(agentSpecialization != nullptr);
            return std::make_unique<_Behaviour>(agentSpecialization);
        }
    }
};

}
