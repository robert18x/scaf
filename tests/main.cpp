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
        std::cout << "Got AclMessage: " << m.content << std::endl;
        return {};
    }

    virtual std::future<scaf::Behaviour<_Agent>*> start() override {
        std::promise<scaf::Behaviour<_Agent>*> p;
        p.set_value(this);
        std::cout << "ResponseWithTemperatureBehaviour starts" << std::endl;
        data = "prepared data";
        return p.get_future();
    }

    bool isFinished() override {
        // TODO
        return true;
    }

    std::string data;
};


class DefaultCommunicationHandler : public scaf::CommunicationHandler {
public:
    void send(const std::string& data) override {
        std::cout << data << std::endl;  // TODO
    }
};

class DefaultErrorHandler : public scaf::ErrorHandler {
public:
    void handle(const scaf::Error& error) noexcept override {
        std::cerr << magic_enum::enum_name(error.getRetCode()) << error.getMessage() << std::endl;
    }
};

class MyAgent : public scaf::Agent<ResponseWithTemperatureBehaviour<MyAgent>, DefaultCommunicationHandler, DefaultErrorHandler> {
public:
    explicit MyAgent(const std::string& name) : Super(name) {}
    void work() {
        std::future behaviour = startConversation("other_agent");
        ResponseWithTemperatureBehaviour<MyAgent>* behv = static_cast<ResponseWithTemperatureBehaviour<MyAgent>*>(behaviour.get());
        std::cout << behv->data << std::endl;
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
