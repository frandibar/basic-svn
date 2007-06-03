// Container.h

#ifndef CONTAINER_H_INCLUDED
#define CONTAINER_H_INCLUDED

#include <fstream>
#include <string>

using std::fstream;
using std::string;

class Container
{
public:
    Container() : _isOpen(false) {};
    
    bool create(const string& a_Filename);
    bool destroy();

    bool open(const string& a_Filename);
    bool close();
    
    long int append(std::ifstream& is);
    bool get(long int offset, std::ofstream& fs);

private:
    bool    _isOpen;
    fstream _fstream;
    string  _filename;
};


#endif

