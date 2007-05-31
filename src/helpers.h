// helpers.h

#include <string>
#include <sstream>

std::string zeroPad(int number, int ndigits);

template<typename T> 
std::string toString(T n) 
{
    std::ostringstream os; 
    os << n; 
    return os.str();
}

