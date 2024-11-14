#pragma once

#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <span>
#include <string>
#include <expected>
#include <cctype>
#include <ranges>

#include "AclMessage.h"
#include "utils.h"
#include "Error.h"

namespace scaf {

class JsonSerializer {
public:
    std::expected<AclMessage, Error> deserialize(std::span<char> data) {
        using namespace scaf::utils;
        try {
            nlohmann::json json = nlohmann::json::parse(data);
            if (auto it = json.find("language"); it == json.end() or not compareStringsLowercase(it.value().get<std::string>(), language))
                return std::unexpected(Error(RetCode::deserialization_error, fmt::format("Missing or invalid language type. Currently only {} is supported", language)));

            if (auto it = json.find("encoding"); it == json.end() or not compareStringsLowercase(it.value().get<std::string>(), encoding))
                return std::unexpected(Error(RetCode::deserialization_error, fmt::format("Missing or invalid encoding type. Currently only {} is supported", encoding)));

            return json.get<AclMessage>();
        } catch (const std::exception& e) {
            return std::unexpected(Error(RetCode::deserialization_error, fmt::format("Occured error while deserialization, error: {}", e.what())));
        }
    }

    std::expected<std::string, Error> serialize(AclMessage& message) {
        try {
            message.encoding = encoding;
            message.language = language;
            nlohmann::json json = message;
            return json.dump();
        } catch (const std::exception& e) {
            return std::unexpected(Error(RetCode::serialization_error, e.what()));
        }
    }

    static inline constexpr std::string encoding = "utf-8";
    static inline constexpr std::string language = "json";
};

}
