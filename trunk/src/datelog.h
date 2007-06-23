#ifndef DATELOG_H_INCLUDED
#define DATELOG_H_INCLUDED

#include <fstream>
#include <string>

using std::string;
using std::fstream;

class DateLog { 
public:
    DateLog() : _isOpen(false) {};
    
    bool create(const string& a_Filename);
    bool destroy();

    bool open(const string& a_Filename);
    bool close();
    
    long int append(const string& a_Username, const string& a_Date, const string& a_Version, const string&  a_Filename);
    bool showDate(const string& a_Date, int offset);
    bool show(int offset);
    bool showAll();
private:
    bool    _isOpen;
    fstream _fstream;
    string  _filename;
};
#endif

