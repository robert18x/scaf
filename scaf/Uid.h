#pragma once
#include "AclMessage.h"

#include <utility>

namespace scaf {

struct UniqueConversationId {
private:
    using conversationId_t = decltype(AclMessage::conversationId);
    using sender_t = decltype(AclMessage::sender);
public:
    constexpr UniqueConversationId(const conversationId_t& conversationId, const sender_t& sender) : sender(sender), conversationId{conversationId} {}
    constexpr auto operator<=>(const UniqueConversationId&) const = default;

    sender_t sender;
    conversationId_t conversationId;
};

}