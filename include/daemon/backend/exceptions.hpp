#ifndef GEKKOFS_DB_EXCEPTIONS_HPP
#define GEKKOFS_DB_EXCEPTIONS_HPP

#include <string>
#include <stdexcept>

namespace gkfs {
namespace metadata {

class DBException : public std::runtime_error {
public:
    explicit DBException(const std::string& s) : std::runtime_error(s){};
};

class NotFoundException : public DBException {
public:
    explicit NotFoundException(const std::string& s) : DBException(s){};
};

class ExistsException : public DBException {
public:
    explicit ExistsException(const std::string& s) : DBException(s){};
};

} // namespace metadata
} // namespace gkfs

#endif // GEKKOFS_DB_EXCEPTIONS_HPP
