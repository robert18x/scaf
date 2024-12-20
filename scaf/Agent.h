#pragma once
#include <cassert>
#include <concepts>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
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
    explicit Agent(const std::string& name)
        : name(name)
        , conversationHandler(this) {}

    explicit Agent(const std::string& name, _CommunicationHandler&& communicationHandler, _ErrorHandler&& errorHandler)
        : name(name)
        , communicationHandler(std::move(communicationHandler))
        , errorHandler(std::move(errorHandler))
        , conversationHandler(this) {}

    virtual ~Agent() = default;
    Agent(const Agent&) = delete;
    Agent(Agent&&) = delete;

    void handleData(Data data) {
        auto ret = utils::safeCall([&]{
            std::expected message = serializer.deserialize(data.data);
            if (message.has_value())
                conversationHandler.handleMessage(message.value());
            else
                errorHandler.handle(message.error());
        });
        if (!ret) {
            errorHandler.handle(ret.error());
        }
    }

    void start() {
        std::jthread communicationThread([&] { listenForMessages(); });
        work();
    }

    using AgentBehaviour = _Behaviour;

protected:
    virtual std::shared_ptr<_Behaviour> createConversation(const decltype(AclMessage::receiver)& receiver) {
        return conversationHandler.createNewConversation(receiver);
    }

    // it is recommended to use sendMessage member function over direct communicationHandler call
    std::expected<void, Error> sendMessage(AclMessage&& message, const _Behaviour& behaviour) {
        UniqueConversationId uid = behaviour.getUid();
        message.receiver = uid.sender;
        message.conversationId = uid.conversationId;
        return send(uid.sender, std::move(message));
    }

    std::expected<void, Error> sendMessage(const std::string& to, AclMessage&& message) {
        message.conversationId = conversationHandler.generateConversationId();
        return send(to, std::move(message));
    }

    friend class ConversationHandler<Agent>;
    friend _Behaviour;

    using Super = Agent<_Behaviour, _CommunicationHandler, _ErrorHandler>;

    const std::string name;
    JsonSerializer serializer;
    _CommunicationHandler communicationHandler;
    _ErrorHandler errorHandler;
    ConversationHandler<Agent> conversationHandler;

private:

    virtual void work() = 0;

    virtual bool finnished() = 0;

    std::expected<void, Error> send(const std::string& to, AclMessage&& message) {
        message.sender = name;
        std::expected status = serializer.serialize(message)
            .and_then([&](const std::string& data){ return communicationHandler.send(to, data); });

        if (not status.has_value())
            errorHandler.handle(status.error());

        return status;
    }

    void listenForMessages() {
        while(not finnished()) {
            std::expected<Data, Error> received = communicationHandler.receive();
            if (received.has_value())
                handleData(received.value());
            else
                errorHandler.handle(received.error());
        }
    }

    virtual std::shared_ptr<_Behaviour> createBehaviour(UniqueConversationId uid) {
        static_assert(std::derived_from<typename _Behaviour::Agent, Agent>);
        if constexpr (std::is_same_v<typename _Behaviour::Agent, std::remove_cvref_t<decltype(*this)>>) {
            return std::make_shared<_Behaviour>(this, uid);
        } else {
            auto* agentSpecialization = static_cast<typename _Behaviour::Agent*>(this);
            return std::make_shared<_Behaviour>(agentSpecialization, uid);
        }
    }
};

}
