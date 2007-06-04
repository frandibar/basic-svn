// helpers.cpp

#include "helpers.h"

#include <cstdlib>      // rand
#include <fstream>
#include <sys/stat.h>

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

t_filetype getFiletype(const std::string& filename)
{
    struct stat ss;
    if (stat(filename.c_str(), &ss) == -1)
        return INVALID; // error, file does not exist

    if ((ss.st_mode & S_IFMT) == S_IFDIR)
        return DIRECTORY;

    // determine if filename is a text or binary file
    // by reading first 512 bytes and searching for chars < 30
    // text files have majority of bytes > 30 while binary files don't
    std::ifstream is(filename.c_str());
    if (!is) return INVALID; // error, could not open file

    int bytesRead = 0;
    int nChars    = 0;
    int nNonchars = 0;
    while (!is.eof() && bytesRead++ < 512) {
        char c;
        is >> c;
        (c > 30) ? nChars++ : nNonchars++;
    }
    is.close();
    return (nChars > nNonchars) ? TEXT : BINARY;
}

