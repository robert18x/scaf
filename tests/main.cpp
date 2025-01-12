#include <fmt/format.h>

#include <cassert>
#include <chrono>
#include <concepts>
#include <iostream>
#include <magic_enum.hpp>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>

#include "Agent.h"
#include "Behaviour.h"
#include "JsonSerializer.h"
#include "Uid.h"

template <typename _Agent>
class ResponseWithTemperatureBehaviour : public scaf::Behaviour<_Agent> {
public:
    explicit ResponseWithTemperatureBehaviour(_Agent* agent, scaf::UniqueConversationId uid) : scaf::Behaviour<_Agent>(agent, uid) {}

    using scaf::Behaviour<_Agent>::agent;

    std::expected<void, scaf::Error> handleReceivedMessage(const scaf::AclMessage& m) override {
        std::cout << agent->name << ": " << "Got AclMessage - " << m.content << std::endl;
        agent->setFinished();
        return {};
    }

    bool isFinished() override {
        // TODO
        return true;
    }

    std::string data;
};


class DefaultCommunicationHandler : public scaf::CommunicationHandler {
public:
    std::expected<void, scaf::Error> send([[maybe_unused]] const std::string& to, const std::string& data) override {
        std::cout << data << std::endl;  // TODO
        return std::expected<void, scaf::Error>();
    }

    std::expected<scaf::Data, scaf::Error> receive() override {
        scaf::AclMessage message{
            .performative = scaf::Performative::propose,
            .sender = "test_agent",
            .receiver = "agent",
            .content = "Data",
            .language = scaf::JsonSerializer::language,
            .encoding = scaf::JsonSerializer::encoding,
            .protocol = "cnp",
            .conversationId = 1,
        };
        nlohmann::json json = message;
        return scaf::Data{
            .from = "test_agent",
            .data = json.dump(),
        };
    }

    void stop() override {}
};

class DefaultErrorHandler : public scaf::ErrorHandler {
public:
    void handle(const scaf::Error& error) noexcept override {
        std::cerr << magic_enum::enum_name(error.getRetCode()) << ": " << error.getMessage() << std::endl;
    }
};

class MyAgent : public scaf::Agent<ResponseWithTemperatureBehaviour<MyAgent>, DefaultCommunicationHandler, DefaultErrorHandler> {
public:
    explicit MyAgent(const std::string& name) : Super(name) {}

private:
    void work() override {
        auto behaviour = createConversation("other_agent");
        std::cout << behaviour->data << std::endl;
        setFinished();
    }

    bool finished() { 
        static bool firstTime = true;
        if (firstTime) {
            firstTime = false;
            return false;
        } else {
            return true;
        }
    }
};


std::function<void(std::string)> handler = nullptr;

void event() {
    static int i = 0;

    using namespace scaf;
    AclMessage message{
        .performative = Performative::inform,
        .receiver = "other",
        .content = fmt::format("Event {}", ++i),
        .language = "json",
        .encoding = "utf-8",
        .ontology = "FIPA ACL",
        .protocol = "CNP",
    };

    nlohmann::json json = message;
    std::string data = json.dump();

    if (handler) {
        handler(data);
    }
}


void testJsonSerialization() {
    using namespace scaf;

    {
        AclMessage message{
            .performative = Performative::inform,
            .sender = "sender",
            .receiver = "receiver",
            .replyTo = "replayTo",
            .content = "content",
            .language = "json",
            .encoding = "utf-8",
            .ontology = "FIPA ACL",
            .protocol = "CNP",
            .conversationId = 1234567890u,
            .replyWith = "replayWith",
            .inReplyTo = "inReplayTo",
            .replyBy = std::chrono::system_clock::now()
        };
        
        JsonSerializer serializer;

        std::expected<std::string, Error> serialized = serializer.serialize(message);
        assert(serialized.has_value());

        std::expected<AclMessage, Error> deserialized = serializer.deserialize(serialized.value());
        assert(deserialized.has_value());
        
        AclMessage message2 = deserialized.value();
        assert(message == message2);
    }

    {
        AclMessage message = AclMessageBuilder{
            .performative = Performative::inform,
            .replyTo = "replayTo",
            .content = "content",
            .protocol = "CNP",
            .inReplyTo = "inReplayTo",
            .replyBy = std::chrono::system_clock::now()
        };
        
        JsonSerializer serializer;

        std::expected<std::string, Error> serialized = serializer.serialize(message);
        assert(serialized.has_value());

        std::expected<AclMessage, Error> deserialized = serializer.deserialize(serialized.value());
        assert(deserialized.has_value());
        
        AclMessage message2 = deserialized.value();
        assert(message == message2);
    }
}


int main() {
    testJsonSerialization();

    std::unique_ptr<MyAgent> myAgent;
    myAgent = std::make_unique<MyAgent>("MyAgent");
    handler = [&] (const std::string& data) { myAgent->handleData(scaf::Data{.from={}, .data=data}); };
    myAgent->startListening();
    event();
}
