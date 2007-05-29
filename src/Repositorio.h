// Repositorio.h

#ifndef REPOSITORIO_H_INCLUDED
#define REPOSITORIO_H_INCLUDED

#include "arbolbmas.h"
#include "ConfigData.h"
#include "VersionManager.h"
#include "User.h"

#include <list>

class Repositorio
{
public:
    Repositorio(const string& a_Almacen, const string& a_Name);
    ~Repositorio() {};

    bool create();
    bool open();
    bool addFile(const string& a_Filename, const string& a_Username, const string& a_Password);
    bool userExists(const string& a_Username) const;
    bool validatePassword(const string& a_Username, const string& a_Password) const;
    bool validateUser    (const string& a_Username, const string& a_Password) const;
    bool addUser(const string& a_Username, const string& a_Password, const string& a_Fullname);
    bool removeUser(const string& a_Username);

    // getters
    string getName() const { return _name; }
    std::list<User> getListOfUsers() const { return _lUsers; }

private:
    Repositorio();

    // member variables
    int             _version;
    string          _name;
    string          _almacen;
    std::list<User> _lUsers;
    VersionManager  _versionManager;
};

#endif

