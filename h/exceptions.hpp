#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <string>

class TextureException {
public:
    TextureException(const std::string& message) : m_message(message) {}

    const std::string& message() const { return m_message; }

private:
    std::string m_message;
};

#endif