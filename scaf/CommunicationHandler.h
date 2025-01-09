#pragma once

#include <expected>
#include <string>

#include "Error.h"

namespace scaf {
struct Data {
    std::string from;
    std::string data;
};

class CommunicationHandler {
public:

    virtual ~CommunicationHandler() = default;
    virtual std::expected<void, Error> send(const std::string& to, const std::string& data) = 0;
    virtual std::expected<Data, Error> receive() = 0;
    virtual void stop() = 0;
};

}
