#pragma once

#include "utils/nlohman_json_serializers.h"
#include "utils/safeCall.h"

#include <ranges>
#include <string_view>

namespace scaf::utils {

constexpr bool compareStringsLowercase(std::string_view first, std::string_view second) {
    auto toLower = std::views::transform([](auto c) { return std::tolower(c); });
    return std::ranges::equal(first | toLower, second | toLower);
}

}

