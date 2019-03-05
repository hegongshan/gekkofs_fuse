#ifndef IFS_DB_EXCEPTIONS_HPP
#define IFS_DB_EXCEPTIONS_HPP

#include <string>
#include <stdexcept>

class DBException: public std::runtime_error {
    public:
        DBException(const std::string & s) : std::runtime_error(s) {};
};

class NotFoundException: public DBException {
    public:
        NotFoundException(const std::string & s) : DBException(s) {};
};

#endif //IFS_DB_EXCEPTIONS_HPP
