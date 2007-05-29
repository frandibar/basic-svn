// Container.cpp

#include "Container.h"

using std::ios;

bool Container::create(const string& a_Name)
{
    _fstream.open(a_Name.c_str(), ios::out | ios::in | ios::binary);

	if (!_fstream) {
		_fstream.open(a_Name.c_str(), ios::out | ios::binary);
		_fstream.close();
		_fstream.open(a_Name.c_str(), ios::in | ios::out | ios::binary);
	}

    _isOpen = _fstream.is_open();
    return _isOpen;
}

bool Container::open(const string& a_Name)
{
    _fstream.open(a_Name.c_str(), ios::binary | ios::in | ios::out);
    _isOpen = _fstream.is_open();
    return _isOpen;
}

long int Container::append(std::ifstream is)
// returns the offset where is was written    
{
    if (!_fstream.is_open())
        return -1;

    _fstream.seekg(ios::end, ios::beg);
    _fstream.seekp(ios::end, ios::beg);
    long int pos = _fstream.tellg();
   
    // write length of is
    _fstream.write((char*)&pos, sizeof(pos));

    // write is into container
    _fstream << is;
    return pos;
}


bool Container::get(long int offset, fstream& fs) 
// offset points to a long int containing the length of the block of text
// read that block and return it     
{
    return true; 
}
