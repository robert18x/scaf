#pragma once
#include <cctype>
#include <chrono>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace scaf::utils {

bool compareStringsLowercase(std::string_view first, std::string_view second) {
    std::string firstLowercase(first), secondLowercase(second);
    std::ranges::for_each(firstLowercase, [](char& c) { c = std::tolower(c); });
    std::ranges::for_each(secondLowercase, [](char& c) { c = std::tolower(c); });
    return firstLowercase == secondLowercase;
}

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
