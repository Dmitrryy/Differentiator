#include "LogEntry.h"

#include <chrono>
#include <fstream>
#include <iomanip>

void makeLogEntry (const std::string& _message, size_t _line) {
    auto now = std::chrono::system_clock::now();

    static size_t current_entry = 1;
    static bool is_first = true;
    static auto beg =  std::chrono::system_clock::now();

    std::ofstream out ("Log.txt", std::ios_base::app);

    if (is_first) {
        out << std::endl;
        auto current_time = std::chrono::system_clock::to_time_t(beg);
        out << "*************************" << std::endl << std::ctime(&current_time) << "*************************";
        out << std::endl;

        is_first = false;
    }

    out << '[' << current_entry << "] " << std::setw(5) << _line << ": "
    << std::chrono::duration_cast<std::chrono::seconds>(now - beg).count() << "s -- " << _message;
    out << std::endl;

    current_entry++;
    out.close();
}

void makeLogError (const std::string& _message, size_t _line) {
    auto now = std::chrono::system_clock::now();

    static size_t current_entry = 1;
    static auto beg =  std::chrono::system_clock::now();

    std::ofstream out ("Log.txt", std::ios_base::app);


    out << '[' << current_entry << "] " << std::setw(5) << _line << ": ERROR!!! "
    << std::chrono::duration_cast<std::chrono::seconds>(now - beg).count()<< "s -- " << _message;
    out << std::endl;

    out.close();
}