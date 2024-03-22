#include <fmt/format.h>

#include <chrono>
#include <concepts>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "include/Agent.h"
#include "include/Behaviour.h"
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>


template <typename _Agent>
class ResponseWithTemperatureBehaviour : public scaf::Behaviour<_Agent> {
public:
    explicit ResponseWithTemperatureBehaviour(_Agent* agent) : scaf::Behaviour<_Agent>(agent) {}

    using scaf::Behaviour<_Agent>::agent;

    void handleReceivedMessage(const scaf::AclMessage& m) override {
        std::cout << "Got AclMessage: " << m.content << std::endl;
    }

    virtual void start() override {
        std::cout << "ResponseWithTemperatureBehaviour starts" << std::endl;
    }

    bool isFinished() override {
        // TODO
        return true;
    }
};


class DefaultCommunicationHandler : public scaf::CommunicationHandler {
public:
    void send(const std::string& data) override {
        std::cout << data << std::endl;  // TODO
    }
};

class MyAgent : public scaf::Agent<ResponseWithTemperatureBehaviour<MyAgent>, DefaultCommunicationHandler> {
public:
    explicit MyAgent(const std::string& name) : scaf::Agent<ResponseWithTemperatureBehaviour<MyAgent>, DefaultCommunicationHandler>(name) {}
    void work() {
        startConversation("other_agent");
    }
};

std::unique_ptr<MyAgent> myAgent;

void handler(std::span<char> data) {
    if (myAgent != nullptr) myAgent->handleData(data);
}

void set_event_handler(void (*on_data_handler)(std::span<char>)) {
    using namespace scaf;
    AclMessage message{
        .performative = Performative::inform,
        .sender = "this",
        .receiver = "other",
        .content = "Hello World",
        .language = "json",
        .encoding = "utf8",
        .ontology = "FIPA ACL",
        .protocol = "CNP",
        .conversationId = 0,
    };
    nlohmann::json json = message;
    std::string data = json.dump();
    on_data_handler(std::span<char>(data.data(), data.size()));
}

int main() {
    myAgent = std::make_unique<MyAgent>("MyAgent");
    set_event_handler(handler);
    myAgent->work();
}
