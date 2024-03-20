#include <fmt/format.h>

#include <cassert>
#include <chrono>
#include <concepts>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>

namespace nlohmann {
template <typename T>
struct adl_serializer<std::optional<T>> {
    static void to_json(json& j, const std::optional<T>& opt) {
        if (opt.has_value()) {
            j = *opt;
        } else {
            j = nullptr;
        }
    }

    static void from_json(const json& j, std::optional<T>& opt) {
        if (!j.is_null()) {
            opt = j.template get<T>();
        } else {
            opt = std::nullopt;
        }
    }
};
}

namespace nlohmann {
template <typename Clock, typename Duration>
struct adl_serializer<std::chrono::time_point<Clock, Duration>> {
    static void to_json(json& j, const std::chrono::time_point<Clock, Duration>& tp) {
        j = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
    }

    static void from_json(const json& j, std::chrono::time_point<Clock, Duration>& tp) {
        if (j.is_null()) {
            auto ms_since_epoch = j.template get<std::int64_t>();
            Duration since_epoch = std::chrono::duration_cast<Duration>(std::chrono::microseconds(ms_since_epoch));
            tp = std::chrono::time_point<Clock, Duration>(since_epoch);
        }
    }
};
}

namespace scaf {  // Smart Contracting Agents Framework

enum class Performative {
    Accept_Proposal,
    Agree,
    Cancel,
    Call_For_Proposal,
    Confirm,
    Disconfirm,
    Failure,
    Inform,
    Inform_If,
    Inform_Ref,
    Not_Understood,
    Propagate,
    Propose,
    Proxy,
    Query_If,
    Query_Ref,
    Refuse,
    Reject_Proposal,
    Request,
    Request_When,
    Request_Whenever,
    Subscribe,
};

NLOHMANN_JSON_SERIALIZE_ENUM(Performative, {// clang-format off
    {Performative::Accept_Proposal, "Accept_Proposal"},
    {Performative::Agree, "Agree"},
    {Performative::Cancel, "Cancel"},
    {Performative::Call_For_Proposal, "Call_For_Proposal"},
    {Performative::Confirm, "Confirm"},
    {Performative::Disconfirm, "Disconfirm"},
    {Performative::Failure, "Failure"},
    {Performative::Inform, "Inform"},
    {Performative::Inform_If, "Inform_If"},
    {Performative::Inform_Ref, "Inform_Ref"},
    {Performative::Not_Understood, "Not_Understood"},
    {Performative::Propagate, "Propagate"},
    {Performative::Propose, "Propose"},
    {Performative::Proxy, "Proxy"},
    {Performative::Query_If, "Query_If"},
    {Performative::Query_Ref, "Query_Ref"},
    {Performative::Refuse, "Refuse"},
    {Performative::Reject_Proposal, "Reject_Proposal"},
    {Performative::Request, "Request"},
    {Performative::Request_When, "Request_When"},
    {Performative::Request_Whenever, "Request_Whenever"},
    {Performative::Subscribe, "Subscribe"}
})  // clang-format on

struct AclMessage {
    Performative performative;
    std::string sender;
    std::string receiver;
    std::optional<std::string> replayTo = std::nullopt;
    nlohmann::json content;
    std::string language;
    std::string encoding;
    std::optional<std::string> ontology = std::nullopt;
    std::string protocol;
    std::uint64_t conversationId;
    std::optional<std::string> replayWith = std::nullopt;
    std::optional<std::string> inReplayTo = std::nullopt;
    std::optional<std::chrono::system_clock::time_point> replayBy = std::nullopt;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AclMessage, performative, sender, receiver, replayTo, content, language, encoding, ontology, protocol, conversationId,
                                   replayWith, inReplayTo, replayBy);

namespace utils {
    bool compareStringsLowercase(std::string_view first, std::string_view second) {
        std::string firstLowercase(first), secondLowercase(second);
        std::ranges::for_each(firstLowercase, [](char& c){ c = std::tolower(c); });
        std::ranges::for_each(secondLowercase, [](char& c){ c = std::tolower(c); });
        return firstLowercase == secondLowercase;
    }
}

class ScafError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    virtual const char* getErrorTypeName() {
        return errorTypeName.c_str();
    }

private:
    const static inline std::string errorTypeName = "ScafError";
};

class SerializationError : public ScafError {
public:
    using ScafError::ScafError;

    const char* getErrorTypeName() override {
        return errorTypeName.c_str();
    }

private:
    const static inline std::string errorTypeName = "SerializationError";
};

class JsonSerializer {
public:
    AclMessage deserialize(std::span<char> data) {
        try {
            nlohmann::json json = nlohmann::json::parse(data);
            if (not json.contains("language") or json.at("language") != language)
                throw SerializationError("Missing or invalid language type. Currently only Json is supported");

            if (not json.contains("encoding") or not utils::compareStringsLowercase(json.at("encoding").get<std::string>(), encoding))
                throw SerializationError("Missing or invalid encoding type. Currently only utf8 is supported");

            return json.get<AclMessage>();
        } catch (const SerializationError& e) {
            throw e;
        } catch (const std::exception& e) {
            throw SerializationError(fmt::format("Occured error while deserialization, error: {}", e.what()));
        }
    }

    std::string serialize(AclMessage& message) {
        try {
            message.encoding = encoding;
            message.language = language;
            nlohmann::json json = message;
            return json.dump();
        } catch (const std::exception& e) {
            throw SerializationError(fmt::format("Occured error while serialization, error: {}", e.what()));
        }
    }

private:
    static inline const std::string encoding = "utf8";
    static inline const std::string language = "json";
};

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
            auto conversation = createNewConversation(uid);
            [[maybe_unused]] auto [iterator, inserted] = activeConversations.emplace(uid, std::move(conversation));
            assert(inserted == true);
            handleConversation(uid, *iterator->second, message);
        }
    }

private:
    auto createNewConversation(const UniqueConversationId& uid) {
        auto behaviour = correspondingAgent->createBehaviour();
        behaviour->setUid(uid);
        return behaviour;
    }

    void handleConversation(const UniqueConversationId& uid, Conversation& conversation, const AclMessage& message) {
        try {
            conversation.handleMessage(message);
        } catch (...) {  // unhandled exception - invalid conversation
            removeConversation(uid);
        }
        if (conversation.isFinished())  // if finnished also remove conversation
            removeConversation(uid);
    }

    void removeConversation(const UniqueConversationId& uid) {
        activeConversations.erase(uid);
    }

    std::map<UniqueConversationId, std::unique_ptr<Conversation>> activeConversations;
    _Agent* correspondingAgent;
};

class CommunicationHandler {
public:
    virtual void send(std::string data) = 0;
};

class DefaultCommunicationHandler : public CommunicationHandler {
public:
    void send(std::string data) override {
        std::cout << data << std::endl;  // TODO
    }
};

template <typename _Behaviour, typename _CommunicationHandler = DefaultCommunicationHandler>
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

    _CommunicationHandler& getCommunicationHandler() {
        return communicationHandler;
    }

    void sendMessage(AclMessage&& message, UniqueConversationId uid) {
        message.sender = name;
        message.receiver = uid.second;
        message.conversationId = uid.first;
        std::string data = serializer.serialize(message);

        communicationHandler.send(data);
        // message.+
    }

    using AgentBehaviour = _Behaviour;
    friend class ConversationHandler<Agent>;

protected:
    const std::string name;
    JsonSerializer serializer;
    _CommunicationHandler communicationHandler;
    ConversationHandler<Agent> conversationHandler;

private:
    std::unique_ptr<_Behaviour> createBehaviour() {
        static_assert(std::derived_from<typename _Behaviour::Agent, Agent>);
        auto* agentSpecialization = dynamic_cast<typename _Behaviour::Agent*>(this);
        assert(agentSpecialization != nullptr);
        return std::make_unique<_Behaviour>(agentSpecialization);
    }
};

template <typename _Agent>
class Behaviour {
public:
    explicit Behaviour(_Agent* agent) : agent(agent) {}
    virtual ~Behaviour() = default;

    virtual void handleMessage(const AclMessage&) = 0;

    virtual bool isFinished() = 0;
    using Agent = _Agent;

protected:
    _Agent* agent;

private:
    template <typename T>
    friend class ConversationHandler;

    void setUid(UniqueConversationId uid) {
        uniqueConversationId = uid;
    }
    UniqueConversationId uniqueConversationId;
};

}

template <typename _Agent>
class MyBehaviour : public scaf::Behaviour<_Agent> {
public:
    explicit MyBehaviour(_Agent* agent) : scaf::Behaviour<_Agent>(agent) {}

    using scaf::Behaviour<_Agent>::agent;

    void handleMessage(const scaf::AclMessage& m) override {
        std::cout << "Got AclMessage: " << m.content << std::endl;
        agent->getCommunicationHandler();
    }

    bool isFinished() override {
        // TODO
        return true;
    }
};

class MyAgent : public scaf::Agent<MyBehaviour<MyAgent>> {
public:
    explicit MyAgent(const std::string& name) : scaf::Agent<MyBehaviour<MyAgent>>(name) {}
};

std::unique_ptr<MyAgent> myAgent;

void handler(std::span<char> data) {
    if (myAgent != nullptr) myAgent->handleData(data);
}

void set_event_handler(void (*on_data_handler)(std::span<char>)) {
    using namespace scaf;
    AclMessage message{
        .performative = Performative::Inform,
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
}
