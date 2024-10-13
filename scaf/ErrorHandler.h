#pragma once

#include <string>
#include "Error.h" 

namespace scaf {

class ErrorHandler {
public:
    virtual ~ErrorHandler() = default;
    virtual void handle(const Error& error) noexcept = 0;
};

}
