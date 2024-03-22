#pragma once
#include <exception>
#include <stdexcept>
#include <string>

namespace scaf {

class ScafError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    virtual const char* getErrorTypeName() {
        return errorTypeName.c_str();
    }

private:
    const static inline std::string errorTypeName = "ScafError";
};

class SerializationError : public ScafError {
public:
    using ScafError::ScafError;

    const char* getErrorTypeName() override {
        return errorTypeName.c_str();
    }

private:
    const static inline std::string errorTypeName = "SerializationError";
};
}
