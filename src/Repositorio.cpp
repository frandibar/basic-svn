// Repositorio.cpp

#include "Repositorio.h"
#include "debug.h"

#include <sys/stat.h>
#include <fstream>
#include <ctime>


Repositorio::t_filetype Repositorio::getFiletype(const string& filename)
{
    struct stat ss;
    if (stat(filename.c_str(), &ss) == -1)
        return INVALID; // error, file does not exist

    if ((ss.st_mode & S_IFMT) == S_IFDIR)
        return DIRECTORY;

    // determine if filename is a text or binary file
    // by reading first 512 bytes and searching for chars < 30
    // text files have majority of bytes > 30 while binary files don't
    std::ifstream is(filename.c_str());
    if (!is) return INVALID; // error, could not open file

    int bytesRead = 0;
    int nchars    = 0;
    int nnonchars = 0;
    while (!is.eof() && bytesRead++ < 512) {
        char c;
        is >> c;
        (c > 30) ? nchars++ : nnonchars++;
    }
    is.close();
    return (nchars > nnonchars) ? TEXT : BINARY;
}

// constructor
Repositorio::Repositorio(const string& a_Almacen, const string& a_Name) throw(SVNException) : 
        _version(0), _name(a_Name), _almacen(a_Almacen), _versionManager(a_Almacen, a_Name)
{
    if (!open())
        throw SVNException();
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

bool Repositorio::removeFile(const string& a_Filename, const string& a_Username, const string& a_Password)
{
    // TODO
    return false;
}

bool Repositorio::addFile(const string& a_Filename, const string& a_Username, const string& a_Password)
{
    if (!validateUser(a_Username, a_Password)) return false;

    int ft = getFiletype(a_Filename);

    if (ft == INVALID) 
        return false; // file not found

    if (ft == DIRECTORY)
        // TODO
        ;
    
    time_t date;
    time(&date);
    _versionManager.open();
    if (!_versionManager.addFile(_version, a_Filename, a_Username, date, (ft == TEXT ? 't' : 'b')))
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
    // TODO
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
    debug("creating Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    // create directory where files will be stored
    mkdir((_almacen + "//" + _name).c_str(), 0755);
    bool ret = _versionManager.create();
    if (!ret)
        remove(_name.c_str());
    debug("Repositorio creation " + string((ret) ? "successfull" : "failed") + "\n");
    return ret;
}

bool Repositorio::destroy()
{
    debug("destroying Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    bool ret = _versionManager.destroy();
    debug("Repositorio destroy " + string((ret) ? "successfully" : "failed") + "\n");
    return ret;
}

bool Repositorio::open()
{
    debug("opening Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    bool ret = _versionManager.open();
    debug("Repositorio open " + string((ret) ? "successfull" : "failed") + "\n");
    return ret;
}

