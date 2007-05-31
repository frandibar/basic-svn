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

bool Container::close()
{
    _fstream.close();
    _isOpen = _fstream.is_open();
    return (!_isOpen);
}

long int Container::append(std::ifstream& is)
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
    int amount = 0;
    int bufferSize = 100;
    char* buf = new char[bufferSize * sizeof(char)];
    if (!buf) return -1;

    do {
        is.read(buf, bufferSize);
        amount = is.gcount();
        _fstream.write(buf, amount);
    } while (amount == bufferSize);
    
    delete buf;
    return pos;
}


bool Container::get(long int offset, fstream& fs) 
// offset points to a long int containing the length of the block of text
// read that block and return it     
{
    if(!_fstream.is_open()) return false;

    _fstream.seekg(offset, ios::beg);
    _fstream.seekp(offset, ios::beg);

    // write is into container
    int bufferSize = 100;
    char* buf = new char[bufferSize * sizeof(char)];
    if (!buf) return -1;

    int fileSize = 0;
    int bytesRead = 0;
    _fstream.read((char*)&fileSize, sizeof(fileSize));
    while (bytesRead < fileSize) {
        _fstream.read(buf, bufferSize);
        int amount = _fstream.gcount();
        fs.write(buf, amount);
        bytesRead += amount;
    }
    
    delete buf;
    return true; 
}

