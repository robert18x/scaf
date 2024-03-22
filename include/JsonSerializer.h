#pragma once

#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <span>
#include <string>

#include "AclMessage.h"
#include "Exceptions.h"
#include "utils.h"

namespace scaf {

class JsonSerializer {
public:
    AclMessage deserialize(std::span<char> data) {
        try {
            nlohmann::json json = nlohmann::json::parse(data);
            if (not json.contains("language") or json.at("language") != language)
                throw SerializationError("Missing or invalid language type. Currently only Json is supported");

            if (not json.contains("encoding") or not utils::compareStringsLowercase(json.at("encoding").get<std::string>(), encoding))
                throw SerializationError("Missing or invalid encoding type. Currently only utf8 is supported");

            return json.get<AclMessage>();
        } catch (const SerializationError& e) {
            throw e;
        } catch (const std::exception& e) {
            throw SerializationError(fmt::format("Occured error while deserialization, error: {}", e.what()));
        }
    }

    std::string serialize(AclMessage& message) {
        try {
            message.encoding = encoding;
            message.language = language;
            nlohmann::json json = message;
            return json.dump();
        } catch (const std::exception& e) {
            throw SerializationError(fmt::format("Occured error while serialization, error: {}", e.what()));
        }
    }

private:
    static inline const std::string encoding = "utf8";
    static inline const std::string language = "json";
};

}
