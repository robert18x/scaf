#pragma once
#include <nlohmann/json.hpp>

#include <chrono>
#include <optional>

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

template <typename Rep, typename Period>
struct adl_serializer<std::chrono::duration<Rep, Period>> {
    static void to_json(json& j, const std::chrono::duration<Rep, Period>& duration) {
        j = duration.count();
    }

    static void from_json(const json& j, std::chrono::duration<Rep, Period>& duration) {
        auto count = j.get<Rep>();
        duration = std::chrono::duration<Rep, Period>(count);
    }
};

}
