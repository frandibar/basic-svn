// helpers.cpp

#include "helpers.h"

std::string zeroPad(int number, int ndigits)
{
    std::string ret(ndigits, '0');
    std::string num = toString<int>(number);

    int j = 0;
    for (int i = ndigits - num.length(); i < ndigits; ++i) {
        ret[i] = num[j];
        j++;
    }
    return ret;
}


