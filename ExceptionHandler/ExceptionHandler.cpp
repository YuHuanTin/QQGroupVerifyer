

#include "ExceptionHandler.h"

#include <fmt/format.h>
#include <fmt/std.h>

void ExceptionHandler::throwExceptionWithOutput(std::string_view ExceptionMessage, std::string_view Where) {
    fmt::println("[{}]: {}", Where, ExceptionMessage);
    m_hasException = true;

    throw std::runtime_error("e");
}

void ExceptionHandler::throwExceptionWithOutput(std::string_view ExceptionMessage, std::string_view Where, int Line) {
    fmt::println("[{} ,line: {}]: {}", Where, Line, ExceptionMessage);
    m_hasException = true;

    throw std::runtime_error("e");
}
