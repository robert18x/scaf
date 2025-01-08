#include <fmt/format.h>

#include <chrono>
#include <concepts>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "Agent.h"
#include "Behaviour.h"
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <magic_enum.hpp>
#include "Uid.h"


template <typename _Agent>
class ResponseWithTemperatureBehaviour : public scaf::Behaviour<_Agent> {
public:
    explicit ResponseWithTemperatureBehaviour(_Agent* agent, scaf::UniqueConversationId uid) : scaf::Behaviour<_Agent>(agent, uid) {}

    using scaf::Behaviour<_Agent>::agent;

    std::expected<void, scaf::Error> handleReceivedMessage(const scaf::AclMessage& m) override {
        std::cout << agent->name << ": " << "Got AclMessage - " << m.content << std::endl;
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
    }

    bool finished() override {
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

int main() {
    std::unique_ptr<MyAgent> myAgent;
    myAgent = std::make_unique<MyAgent>("MyAgent");
    handler = [&] (const std::string& data) { myAgent->handleData(scaf::Data{.from={}, .data=data}); };
    myAgent->start();
    event();
}
