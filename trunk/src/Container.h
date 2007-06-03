// Container.h

#ifndef CONTAINER_H_INCLUDED
#define CONTAINER_H_INCLUDED

#include <fstream>
#include <string>

using std::string;
using std::fstream;

class Container
{
public:
    Container() : _isOpen(false) {};
    bool create(const string& a_Name);
    bool open  (const string& a_Name);
    bool close();
    long int append(std::ifstream& is);
    bool get(long int offset, std::ofstream& fs);

private:
    bool    _isOpen;
    fstream _fstream;

};


#endif

