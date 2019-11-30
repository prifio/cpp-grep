#ifndef KMP_H
#define KMP_H

#include <string>
#include <vector>

class KMP
{
private:
    std::string str;
    std::vector<size_t> pfun;
public:
    KMP(const std::string&);
    size_t next(size_t, char);
    size_t len;
};

#endif // KMP_H
