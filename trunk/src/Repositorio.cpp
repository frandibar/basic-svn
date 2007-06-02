// Repositorio.cpp

#include "debug.h"
#include "Repositorio.h"

#include <sys/stat.h>
#include <fstream>
#include <ctime>


int filetype(const string& filename)
// returns:
// -1 on error 
//  0 if directory 
//  1 if text
//  2 if binary
{
    struct stat ss;
    if (stat(filename.c_str(), &ss) == -1)
        return -1; // error, file does not exist

    if ((ss.st_mode & S_IFMT) == S_IFDIR)
        return 0; // directory

    // determine if filename is a text or binary file
    // by reading first 512 bytes and searching for chars < 30
    // text files have majority of bytes > 30 while binary files don't
    std::ifstream is(filename.c_str());
    if (!is) return -1; // error, could not open file

    int bytesRead = 0;
    int nchars    = 0;
    int nnonchars = 0;
    while (!is.eof() && bytesRead++ < 512) {
        char c;
        is >> c;
        (c > 30) ? nchars++ : nnonchars++;
    }
    is.close();
    return (nchars > nnonchars) ? 1 : 2;
}

// constructor
Repositorio::Repositorio(const string& a_Almacen, const string& a_Name) : _version(0), _name(a_Name), _almacen(a_Almacen)
{
    if (!_versionManager.open()) {
        ;
    }
}

bool Repositorio::validateUser(const string& a_Username, const string& a_Password) const
{
    if (!userExists(a_Username))
        return false;
    
    if (!validatePassword(a_Username, a_Password))
        return false;

    return true;
}

bool Repositorio::validatePassword(const string& a_Username, const string& a_Password) const
{
    std::list<User>::const_iterator it;
    for (it = _lUsers.begin(); it != _lUsers.end(); ++it) {
        if (it->username == a_Username) 
            return (it->password == a_Password);
    }
    return false;
}

bool Repositorio::userExists(const string& a_Username) const
{
    std::list<User>::const_iterator it;
    for (it = _lUsers.begin(); it != _lUsers.end(); ++it) {
        if (it->username == a_Username)
            return true;
    }
    return false;
}

bool Repositorio::addFile(const string& a_Filename, const string& a_Username, const string& a_Password)
{
    if (!validateUser(a_Username, a_Password)) return false;

    int ft = filetype(a_Filename);

    if (ft == -1) 
        return false; // file not found

    if (ft == 0) ; // directory   // TODO
    
    time_t date;
    time(&date);
    //if (!_versionManager.addFile(_version, a_Filename.c_str(), a_Username.c_str()), date, (ft == 1 ? 't' : 'b'))
        return false;
            
    return true;
}

bool Repositorio::addUser(const string& a_Username, const string& a_Password, const string& a_Fullname)
{
    if (userExists(a_Username))
        return false;

    User user;
    user.username = a_Username;
    user.password = a_Password;
    user.fullname = a_Fullname;
    _lUsers.push_back(user);

    return true;
}

bool Repositorio::removeUser(const string& a_Username)
{
    //User u;
    //_lUsers.remove(u);
    std::list<User>::iterator it;
    for (it = _lUsers.begin(); it != _lUsers.end(); ++it) {
        if (it->username == a_Username) {
            //User u = *it;
            //_lUsers.remove(u);
            return true;
        }
    }

    return false; // user not found
}
bool Repositorio::create()
{
    return _versionManager.create();
}

bool Repositorio::open()
{
    return _versionManager.open();
}