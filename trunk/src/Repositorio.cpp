// Repositorio.cpp

#include "Repositorio.h"
#include "debug.h"

#include <ctime>
#include <fstream>
#include <sys/stat.h>


// constructor
Repositorio::Repositorio(const string& a_Almacen, const string& a_Name) :
        _version(0), _name(a_Name), _almacen(a_Almacen), _versionManager(a_Almacen, a_Name), _isOpen(false)
{
}

bool Repositorio::validateUser(const string& a_Username, const string& a_Password) const
{
    return (userExists(a_Username) && validatePassword(a_Username, a_Password));
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

bool Repositorio::removeFileOrDirectory(const string& a_Target, const string& a_Username, const string& a_Password)
{
    if (!_isOpen)
        return false;

    if (!validateUser(a_Username, a_Password)) return false;

    time_t date;
    time(&date);
    if (!_versionManager.open())
        return false;
    if (!_versionManager.removeFileOrDirectory(_version + 1, _name, a_Target, a_Username, date))
        return false;

    if (!_versionManager.close())
        return false;
            
    _version++;
    return true;
}

bool Repositorio::add(const string& a_Target, const string& a_Username, const string& a_Password)
{
    if (!_isOpen)
        return false;

    if (!validateUser(a_Username, a_Password)) return false;

    t_filetype ftype = getFiletype(a_Target);

    if (ftype == INVALID) 
        return false; // file not found
                
    time_t date;
    time(&date);
    if (!_versionManager.open())
        return false;
    if (!_versionManager.add(_version + 1, _name, a_Target, a_Username, date, ftype))
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
    debug("saving repository version to " + toString(_version) + "\n");
    string filename = _almacen + "//" + _name + "//version"; 
    std::ofstream os(filename.c_str());
    if (!os.is_open()) return false;
    os << _version << std::endl;
    os.close();
    return true;
}

bool Repositorio::loadVersion()
{
    debug("loading repository version\n");
    // saves a file named "version" containing the repository version
    string filename = _almacen + "//" + _name + "//version"; 
    std::ifstream is(filename.c_str());
    if (!is.is_open()) return false;
    is >> _version;
    is.close();
    return true;
}

bool Repositorio::get(const string& a_TargetDestiny, const string& a_Target, const string& a_Version, 
                          const string& a_Username, const string& a_Password)
{
    if (!_isOpen)
        return false;

    if (!validateUser(a_Username, a_Password)) 
        return false;

    return _versionManager.get(a_Version, a_Target, _name, a_TargetDestiny);
}

bool Repositorio::getDiff(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_VersionA, const string& a_VersionB, const string& a_Filename)
{
    if (!_isOpen)
        return false;

    if (!validateUser(a_Username, a_Password)) 
        return false;

    return _versionManager.getDiff(is, a_VersionA, a_VersionB, a_Filename, _name);
}

bool Repositorio::getDiffByDate(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_Date)
{
    if (!_isOpen)
        return false;

    if (!validateUser(a_Username, a_Password)) 
        return false;

    return _versionManager.getDiffByDate(is, a_Date);
}


bool Repositorio::getHistory(std::ifstream& is, const string& a_Username, const string& a_Password, const string& a_Filename)
{
    if (!_isOpen)
        return false;

    if (!validateUser(a_Username, a_Password)) 
        return false;

    return _versionManager.getHistory(is, a_Filename);
}


bool Repositorio::getListOfChanges(std::ifstream& is, const string& a_Username, const string& a_Password, int a_Num, bool isAdmin)
{
    if (!_isOpen)
        return false;

    if (isAdmin) {
        if (!userExists(a_Username)) 
            return false;
    }
    else if (!validateUser(a_Username, a_Password)) 
        return false;

    return _versionManager.getListOfChanges(is, a_Username, a_Num);
}

