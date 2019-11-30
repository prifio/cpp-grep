#include "kmp.h"

KMP::KMP(const std::string& s) : str(s),
    len(s.length())
{
    pfun.push_back(0);
    pfun.push_back(0);
    size_t v = 0;
    for (int i = 1; i < s.length(); ++i) {
        while (v > 0 && s[v] != s[i]) {
            v = pfun[v];
        }
        if (s[v] == s[i]) {
            ++v;
        }
        pfun.push_back(v);
    }
}

size_t KMP::next(size_t v, char c) {
    if (v == len) {
        v = pfun[v];
    }
    while (v > 0 && str[v] != c) {
        v = pfun[v];
    }
    if (str[v] == c) {
        ++v;
    }
    return v;
}
