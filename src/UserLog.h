#ifndef USERLOG_H_INCLUDED
#define USERLOG_H_INCLUDED

#include <fstream>
#include <string>

using std::string;
using std::fstream;

class UserLog { 
public:
    UserLog() : _isOpen(false) {};
    
    bool create(const string& a_Filename);
    bool destroy();

    bool open(const string& a_Filename);
    bool close();
    
    long int append(const string& a_User, time_t a_Date, const string& a_Action);
    bool showAction(int offset);

private:
    bool    _isOpen;
    fstream _fstream;
    string  _filename;
};
#endif
