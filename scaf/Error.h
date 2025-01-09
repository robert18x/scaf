#pragma once
#include <string>
#include <string_view>
#include <utility>

namespace scaf {

    enum class RetCode {
        serialization_error,
        deserialization_error,
        generic_error,
        reason,
        no_values,
        invalid_answer,
        expired_message,
        terminating,
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
