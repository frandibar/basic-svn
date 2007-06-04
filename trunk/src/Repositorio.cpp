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
    int nChars    = 0;
    int nNonchars = 0;
    while (!is.eof() && bytesRead++ < 512) {
        char c;
        is >> c;
        (c > 30) ? nChars++ : nNonchars++;
    }
    is.close();
    return (nChars > nNonchars) ? TEXT : BINARY;
}

// constructor
Repositorio::Repositorio(const string& a_Almacen, const string& a_Name) :
        _version(0), _name(a_Name), _almacen(a_Almacen), _versionManager(a_Almacen, a_Name), _isOpen(false)
{
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

    int ftype = getFiletype(a_Filename);

    if (ftype == INVALID) 
        return false; // file not found

    if (ftype == DIRECTORY)
        // TODO
        ;
    
    time_t date;
    time(&date);
    if (!_versionManager.open())
        return false;
    if (!_versionManager.addFile(_version + 1, a_Filename, a_Username, date, (ftype == TEXT ? 't' : 'b')))
        return false;
    if (!_versionManager.close())
        return false;
            
    _version++;
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
    std::list<User>::iterator it;
    for (it = _lUsers.begin(); it != _lUsers.end(); ++it) {
        if (it->username == a_Username) {
            return true;
        }
    }

    return false; // user not found
}

bool Repositorio::create()
{
    if (_isOpen)
        return false;

    debug("creating Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    // create directory where files will be stored
    mkdir((_almacen + "//" + _name).c_str(), 0755);
    _isOpen = saveVersion() && _versionManager.create();
    if (!_isOpen)
        remove(_name.c_str());
    debug("Repositorio creation " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool Repositorio::destroy()
{
    debug("destroying Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    if (!_isOpen)
        _isOpen = open();

    bool ret = _isOpen && _versionManager.destroy();
    ret = ret && (remove((_almacen + "//" + _name).c_str()) == 0);
    debug("Repositorio destroy " + string(ret ? "successfully" : "failed") + "\n");
    return ret;
}

bool Repositorio::open()
{
    if (_isOpen)
        return true;

    debug("opening Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    _isOpen = loadVersion();
    _isOpen = _isOpen && _versionManager.open();
    debug("Repositorio open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool Repositorio::close()
{
    if (!_isOpen)
        return true;

    debug("closing Repositorio '" + _name + "' in Almacen '" + _almacen + "'\n");
    if (!saveVersion())
        return false;
    _isOpen = !_versionManager.close();
    debug("Repositorio close " + string(!_isOpen ? "successfull" : "failed") + "\n");
    return !_isOpen;
}

bool Repositorio::saveVersion()
{
    // saves a file named "version" containing the repository version
    string filename = _almacen + "//" + _name + "//version"; 
    std::ofstream os(filename.c_str());
    if (!os.is_open()) return false;
    os << _version << std::endl;
    os.close();
    return true;
}

bool Repositorio::loadVersion()
{
    // saves a file named "version" containing the repository version
    string filename = _almacen + "//" + _name + "//version"; 
    std::ifstream is(filename.c_str());
    if (!is.is_open()) return false;
    is >> _version;
    is.close();
    return true;
}

