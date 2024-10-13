#pragma once
#include <cctype>
#include <chrono>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <type_traits>
#include <string>
#include <string_view>
#include <concepts>
#include <expected>
#include <source_location>
#include "Error.h"

namespace scaf::utils {

bool compareStringsLowercase(std::string_view first, std::string_view second) {
    auto toLower = std::views::transform([](auto c) { return std::tolower(c); });
    return std::ranges::equal(first | toLower, second | toLower);
}

// template <RetCode errorCode = RetCode::generic_error>
// auto safeCall(std::invocable auto&& callable, std::source_location sl = std::source_location::current()) -> std::expected<std::remove_reference_t<decltype(callable())>, Error> {
//     try {
//         return callable();
//     } catch (const std::exception& e) {
//         return std::unexpected(Error(errorCode, e.what()));
//     } catch (...) {
//         return std::unexpected(Error(errorCode), fmt::format("Occured unknown error while executing function at {}:{}", sl.file_name(), sl.line()));
//     }
// }

}

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
