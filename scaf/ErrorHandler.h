#pragma once
#include "Error.h"

#include <string>

namespace scaf {

class ErrorHandler {
public:
    virtual ~ErrorHandler() = default;
    virtual void handle(const Error& error) noexcept = 0;
};

}
