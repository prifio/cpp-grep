#include "reader.h"

void reader::read_buf() {
    do {
        fin.read(buf, READ_BUF_SIZE);
        if (fin.fail() && !fin.eof())
            throw read_exception("cannot read file");
        len = fin.gcount();
    } while (len == 0 && !fin.eof());
    pt = 0;
}

void reader::check() {
    if (pt == len)
        read_buf();
}

reader::reader(const char* file) {
    valid = false;
    fin.open(file);
    if (!fin.good())
        throw read_exception("Cannot open file");
    valid = true;
    read_buf();
}

char reader::read_char() {
    if (!valid)
        throw read_exception("Reader isn't valid");
    if (pt == len) {
        valid = false;
        throw read_exception("Cannot read file. EOF reached");
    }
    char ans = buf[pt++];
    check();
    return ans;
}

bool reader::is_end() const {
    return pt == len;
}
