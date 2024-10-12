#pragma once
#include <string>
#include <string_view>
#include <utility>

namespace scaf {

    enum class RetCode {
        serialization_error,
        deserialization_error,
    };

    class Error {
    public:
        constexpr Error(RetCode code, const std::string& message) : code(code), message(message) {}
        constexpr Error(RetCode code, std::string&& message) : code(code), message(std::move(message)) {}

        constexpr RetCode getRetCode() const noexcept { return code; }
        constexpr std::string_view getMessage() const noexcept { return message; }

    private:
        RetCode code;
        std::string message;
    };

}