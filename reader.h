#ifndef READER
#define READER

#include <fstream>
#include "constants.h"

struct read_exception : std::exception {
    std::string reasone;
    read_exception(const std::string& s) : reasone(s) {}
};

struct reader {
    reader(const char* file);
    char read_char();
    bool is_end() const;

    private:
    std::ifstream fin;
    void read_buf();
    void check();
    char buf[READ_BUF_SIZE];
    size_t pt;
    size_t len;
    bool valid;
};

#endif
