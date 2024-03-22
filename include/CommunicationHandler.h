#pragma once

#include <string>

namespace scaf {

class CommunicationHandler {
public:
    virtual ~CommunicationHandler() = default;
    virtual void send(const std::string& data) = 0;
};

}
