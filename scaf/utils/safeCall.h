#pragma once
#include "../Error.h"

#include <fmt/format.h>

#include <concepts>
#include <expected>
#include <source_location>
#include <type_traits>

namespace scaf {

namespace details {
    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    template <typename ...T>
    using is_specialization_v = is_specialization<T...>::value;
}

template <RetCode errorCode = RetCode::generic_error>
constexpr auto safeCall(std::invocable auto&& callable, std::source_location sl = std::source_location::current()) ->
    std::conditional_t<details::is_specialization<std::remove_cvref_t<decltype(callable())>, std::expected>::value,
        std::remove_cvref_t<decltype(callable())>,
        std::expected<std::remove_cv_t<decltype(callable())>, Error>
    > {
    try {
        if constexpr (!details::is_specialization<std::remove_cvref_t<decltype(callable())>, std::expected>::value and std::is_same_v<decltype(callable()), void>) {
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
