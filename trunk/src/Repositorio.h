// Repositorio.h

#ifndef REPOSITORIO_H_INCLUDED
#define REPOSITORIO_H_INCLUDED

#include "arbolbmas.h"
#include "ConfigData.h"
#include "SVNException.h"
#include "User.h"
#include "VersionManager.h"

#include <list>

class Repositorio
{
public:
    enum t_filetype { INVALID = -1, DIRECTORY = 0, TEXT, BINARY };

    Repositorio(const string& a_Almacen, const string& a_Name); 
    ~Repositorio() {};

    bool create();
    bool destroy();

    bool open();
    bool close();

    bool addFile   (const string& a_Filename, const string& a_Username, const string& a_Password);
    bool removeFile(const string& a_Filename, const string& a_Username, const string& a_Password);

    bool addUser   (const string& a_Username, const string& a_Password, const string& a_Fullname);
    bool removeUser(const string& a_Username);
    bool userExists(const string& a_Username) const;
    bool validateUser    (const string& a_Username, const string& a_Password) const;
    bool validatePassword(const string& a_Username, const string& a_Password) const;

    // getters
    string getName() const { return _name; }
    std::list<User> getListOfUsers() const { return _lUsers; }

protected:
    t_filetype getFiletype(const string& filename);
    bool saveVersion();
    bool loadVersion();

private:
    Repositorio();

    // member variables
    int             _version;
    string          _name;
    string          _almacen;
    std::list<User> _lUsers;
    VersionManager  _versionManager;
    bool            _isOpen;
};

#endif

