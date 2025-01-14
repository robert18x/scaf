#pragma once
#include "Error.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <cctype>
#include <chrono>
#include <concepts>
#include <expected>
#include <optional>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>

namespace scaf::utils {

constexpr bool compareStringsLowercase(std::string_view first, std::string_view second) {
    auto toLower = std::views::transform([](auto c) { return std::tolower(c); });
    return std::ranges::equal(first | toLower, second | toLower);
}

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

template <RetCode errorCode = RetCode::generic_error>
constexpr auto safeCall(std::invocable auto&& callable, std::source_location sl = std::source_location::current()) -> 
    std::conditional_t<is_specialization<std::remove_cvref_t<decltype(callable())>, std::expected>::value, 
        std::remove_cvref_t<decltype(callable())>,
        std::expected<std::remove_cv_t<decltype(callable())>, Error>
    > {
    try {
        if constexpr (!is_specialization<std::remove_cvref_t<decltype(callable())>, std::expected>::value and std::is_same_v<decltype(callable()), void>) {
            callable();
            return {};
        } else {
            return callable();
        }
    } catch (const std::exception& e) {
        return std::unexpected(Error(errorCode, e.what()));
    } catch (...) {
        return std::unexpected(Error(errorCode, fmt::format("Occured unknown error while executing function at {}:{}", sl.file_name(), sl.line())));
    }
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
        if (not j.is_null()) {
            opt = j.get<T>();
        } else {
            opt = std::nullopt;
        }
    }
};

template <typename Clock, typename Duration>
struct adl_serializer<std::chrono::time_point<Clock, Duration>> {
    static void to_json(json& j, const std::chrono::time_point<Clock, Duration>& tp) {
        j = tp.time_since_epoch().count();
    }

    static void from_json(const json& j, std::chrono::time_point<Clock, Duration>& tp) {
        auto since_epoch = j.get<std::int64_t>();
        tp = std::chrono::time_point<Clock, Duration>(Duration(since_epoch));
    }
};

template <typename Clock, typename Duration>
struct adl_serializer<std::optional<std::chrono::time_point<Clock, Duration>>> {

    using base = adl_serializer<std::chrono::time_point<Clock, Duration>>;

    static void to_json(json& j, const std::optional<std::chrono::time_point<Clock, Duration>>& tp) {
         if (tp.has_value()) {
            base::to_json(j, *tp);
        } else {
            j = nullptr;
        }
    }

    static void from_json(const json& j, std::optional<std::chrono::time_point<Clock, Duration>>& tp) {
        if (not j.is_null()) {
            std::chrono::time_point<Clock, Duration> readed;
            base::from_json(j, readed);
            tp = readed;
        } else {
            tp = std::nullopt;
        }
    }
};
}
