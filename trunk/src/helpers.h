// helpers.h

#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <string>
#include <sstream>

std::string zeroPad(int number, int ndigits);
std::string randomFilename(const std::string& prefix);

enum t_filetype { INVALID = -1, DIRECTORY = 0, TEXT, BINARY };
t_filetype getFiletype(const std::string& filename);

template<typename T> 
std::string toString(T n) 
{
    std::ostringstream os; 
    os << n; 
    return os.str();
}

template<typename T> 
T fromString(const std::string& s) 
{
    std::istringstream is(s); 
    T r; 
    is >> r; 
    return r;
}

#endif
