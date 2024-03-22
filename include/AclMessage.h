#pragma once
#include <chrono>
#include <cinttypes>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>

#include "Performative.h"
#include "utils.h"

namespace scaf {

struct AclMessage {
    Performative performative;
    std::string sender;  // is set automatically, manual setting has no affect
    std::string receiver;
    std::optional<std::string> replayTo = std::nullopt;
    nlohmann::json content;
    std::string language;  // is set automatically, manual setting has no affect
    std::string encoding;  // is set automatically, manual setting has no affect
    std::optional<std::string> ontology = std::nullopt;
    std::string protocol;
    std::uint64_t conversationId;  // is set automatically, manual setting has no affect
    std::optional<std::string> replayWith = std::nullopt;
    std::optional<std::string> inReplayTo = std::nullopt;
    std::optional<std::chrono::system_clock::time_point> replayBy = std::nullopt;

    static_assert(std::is_same_v<decltype(sender), decltype(receiver)>);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AclMessage, performative, sender, receiver, replayTo, content, language, encoding, ontology, protocol, conversationId,
                                   replayWith, inReplayTo, replayBy);

}
