#pragma once

#include <string>

void makeLogEntry (const std::string& _message, size_t _line);
void makeLogError (const std::string& _message, size_t _line);