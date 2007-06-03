// helpers.cpp

#include "helpers.h"

#include <cstdlib>  // rand

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

std::string randomFilename(const std::string& prefix)
// returns a string with a random integer following the prefix    
{
    // a call to seed with a time_t could be used
    // if not, this is quite trivial
    std::string filename(prefix);
    int i = rand();
    return filename + toString(i);
}

