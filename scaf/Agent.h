#pragma once
#include "AclMessage.h"
#include "Behaviour.h"
#include "CommunicationHandler.h"
#include "ConversationHandler.h"
#include "ErrorHandler.h"
#include "JsonSerializer.h"
#include "Uid.h"

#include <cassert>
#include <concepts>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>

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

    virtual ~Agent() {
        setFinished();
        communicationHandler.stop();
    }
    Agent(const Agent&) = delete;
    Agent(Agent&&) = delete;

    void handleData(Data data) {
        auto ret = safeCall([&]{
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

    void startListening() {
        listeningThread = std::jthread([&](std::stop_token stoken) {
            while(not finished and not stoken.stop_requested())
                listenForMessage();
        });
    }

    bool isFinished() {
        return finished;
    }

    void setFinished() {
        finished = true;
    }

    using AgentBehaviour = _Behaviour;

protected:
    virtual std::shared_ptr<_Behaviour> createConversation(const decltype(AclMessage::receiver)& receiver) {
        return conversationHandler.createNewConversation(receiver);
    }

    // it is recommended to use sendMessage member function over direct communicationHandler call
    std::expected<void, Error> sendMessage(const Behaviour<typename AgentBehaviour::Agent>& behaviour, AclMessage&& message) {
        UniqueConversationId uid = behaviour.getUid();
        message.receiver = uid.sender;
        message.conversationId = uid.conversationId;
        return send(std::move(message));
    }

    std::expected<void, Error> sendMessage(AclMessage&& message) {
        message.conversationId = conversationHandler.generateConversationId();
        return send(std::move(message));
    }

    virtual std::string getMessageReceiver(const AclMessage& message) {
        return message.receiver;
    }

    friend class ConversationHandler<Agent>;
    friend _Behaviour;
    friend Behaviour<typename AgentBehaviour::Agent>;

    using Super = Agent<_Behaviour, _CommunicationHandler, _ErrorHandler>;

    const std::string name;
    JsonSerializer serializer;
    _CommunicationHandler communicationHandler;
    _ErrorHandler errorHandler;
    ConversationHandler<Agent> conversationHandler;
    std::jthread listeningThread;
    std::atomic_bool finished = false;

private:

    virtual void work() = 0;

    std::expected<void, Error> send(AclMessage&& message) {
        message.sender = name;
        std::expected status = serializer.serialize(message)
            .and_then([&](const std::string& data){ return communicationHandler.send(getMessageReceiver(message), data); });

        if (not status.has_value())
            errorHandler.handle(status.error());

        return status;
    }

    void listenForMessage() {
        std::expected<Data, Error> received = communicationHandler.receive();

        if (received.has_value()) {
            handleData(received.value());
        } else {
            Error error = received.error();
            if (error.getRetCode() == RetCode::terminating)
                return;

            errorHandler.handle(error);
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
