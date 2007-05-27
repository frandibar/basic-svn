// Repositorio.h

#ifndef REPOSITORIO_H_INCLUDED
#define REPOSITORIO_H_INCLUDED

#include "arbolbmas.h"
#include "ConfigData.h"
#include "VersionManager.h"

#include <list>

class Repositorio
{
public:
    Repositorio(const string& a_Path, const string& a_Name);
    ~Repositorio() {};

    bool addFile(const string& a_Filename, const string& a_Username, const string& a_Password);
    string getName() const { return _name; }
    bool userExists(const string& a_Username) const;
    bool validatePassword(const string& a_Username, const string& a_Password) const;
    bool validateUser(const string& a_Username, const string& a_Password) const;
    bool addUser(const string& a_Username, const string& a_Password, const string& a_Fullname);

private:
    Repositorio();

    // member variables
    string          _path;
    string          _name;
    std::list<User> _lUsers;
    VersionManager  _versionManager;
};

#endif

