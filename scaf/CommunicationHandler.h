#pragma once

#include <string>

namespace scaf {

class CommunicationHandler {
public:
    virtual ~CommunicationHandler() = default;
    virtual std::expected<void, Error> send(const std::string& data) = 0;
    virtual std::expected<std::string, Error> receive() = 0;
};

}
