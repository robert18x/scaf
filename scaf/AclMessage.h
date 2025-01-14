#pragma once
#include "Performative.h"
#include "utils.h"

#include <chrono>
#include <cinttypes>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>

namespace scaf {

struct AclMessage {
    Performative performative;
    std::string sender{};  // is set automatically, manual setting has no affect
    std::string receiver;
    std::optional<std::string> replyTo = std::nullopt;
    nlohmann::json content;
    std::string language{};  // is set automatically, manual setting has no affect
    std::string encoding{};  // is set automatically, manual setting has no affect
    std::optional<std::string> ontology = std::nullopt;
    std::string protocol;
    std::uint64_t conversationId{};  // is set automatically, manual setting has no affect
    std::optional<std::string> replyWith = std::nullopt;
    std::optional<std::string> inReplyTo = std::nullopt;
    std::optional<std::chrono::system_clock::time_point> replyBy = std::nullopt;

    static_assert(std::is_same_v<std::remove_cvref_t<decltype(sender)>, std::remove_cvref_t<decltype(receiver)>>);
    auto operator<=>(const scaf::AclMessage&) const noexcept = default;
};


struct AclMessageBuilder {
    Performative performative;
    std::optional<std::string> replyTo = std::nullopt;
    nlohmann::json content;
    std::optional<std::string> ontology = std::nullopt;
    std::string protocol;
    std::optional<std::string> replyWith = std::nullopt;
    std::optional<std::string> inReplyTo = std::nullopt;
    std::optional<std::chrono::system_clock::time_point> replyBy = std::nullopt;

    operator AclMessage() const {
        return AclMessage{
            .performative = this->performative,
            .sender = {},
            .receiver = {},
            .replyTo = std::move(this->replyTo),
            .content = std::move(this->content),
            .language = {},
            .encoding = {},
            .ontology = std::move(this->ontology),
            .protocol = std::move(this->protocol),
            .conversationId = {},
            .replyWith = std::move(this->replyWith),
            .inReplyTo =std::move(this->inReplyTo),
            .replyBy = std::move(this->replyBy)
        };
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AclMessage, performative, sender, receiver, replyTo, content, language, encoding, ontology, protocol, conversationId, replyWith, inReplyTo, replyBy);

}
