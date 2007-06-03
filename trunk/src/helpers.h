// helpers.h

#include <string>
#include <sstream>

std::string zeroPad(int number, int ndigits);
std::string randomFilename(const std::string& prefix);

template<typename T> 
std::string toString(T n) 
{
    std::ostringstream os; 
    os << n; 
    return os.str();
}

