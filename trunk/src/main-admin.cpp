// main-admin.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>      // getenv

#include "Almacen.h"
#include "debug.h"
#include "User.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;


string CONFIG_FILE;


void addRepository(const string& a_Name);
void addUser(const string& a_Username, const string& a_Pass, const string& a_Fullname, const string& a_Reposit);
void createAlmacen(const string& a_Dir);
bool fileExists(const string& a_Filename);
void removeRepository(const string& a_Name);
void removeUser(const string& a_Reposit, const string& a_Username);
void showHelp(const char* progname);
void showUsers(const string& a_Reposit);

int main(int argc, char** argv)
{
    int c; 
    bool argsok = false;
    while ((c = getopt(argc, argv, "-a:c:e:ho:r:u:")) != -1) {
        switch (c) {
            case 'a': // crear almacen de repositorios
                // -a "nombre directorio"
                argsok = (argc == 3);
                if (argsok) createAlmacen(optarg);
                break;

            case 'c': // agregar repositorio
                // -c "nombre repositorio"
                argsok = (argc == 3);
                if (argsok) addRepository(optarg);
                break;

            case 'e': // eliminar usuario
                // -e usuario "nombre repositorio"
                argsok = (argc == 4);
                if (argsok) removeUser(argv[optind], optarg);
                break;

            case 'h': // mostrar ayuda
                showHelp(argv[0]);
                argsok = true;
                break;

            case 'o': // obtener listado de usuarios
                // -o "nombre repositorio"
                argsok = (argc == 3);
                if (argsok) showUsers(optarg);
                break;

            case 'r': // eliminar repositorio
                // -r "nombre repositorio"
                argsok = (argc == 3);
                if (argsok) removeRepository(optarg);
                break;

            case 'u': // crear usuario
                // -u usuario password "nombre usuario" "nombre repositorio"
                argsok = (argc == 6);
                if (argsok) addUser(optarg, argv[optind], argv[optind + 1], argv[optind + 2]);
                break;
        }
    }

    if (!argsok) {
       cout << "Parametros invalidos." << endl;
       showHelp(argv[0]);
    }
    return 0;
}


void addRepository(const string& a_Name)
{
    Almacen almacen;
    if (!almacen.exists()) {
        cout << "No existe un almacen para alojar el repositorio." << endl;
        return;
    }
    
    // agregar repositorio al archivo de configuracion
    if (!almacen.addRepository(a_Name)) {
        cout << "No se ha creado el repositorio '" << a_Name << "'." << endl
             << "Verifique que no exista otro con el mismo nombre." << endl;
        return;
    }

    cout << "Se ha creado el repositorio '" << a_Name << "' exitosamente." << endl;
}


void createAlmacen(const string& a_Dir) 
{
    Almacen almacen;
    if (almacen.exists()) {
        char r;
        do {
            cout << "Ya existe un almacen. Desea removerlo y crear uno nuevo? (s/n): ";
            cin >> r;
        } while (r != 'n' && r != 'N' && r != 's' && r != 'S');
        if (r == 'n' || r == 'n')
            return;
        else
            almacen.remove();
    }

    if (almacen.create(a_Dir))
        cout << "El almacen '" << a_Dir << "' ha sido creado con exito." << endl;
    else
        cout << "El almacen '" << a_Dir << "' no ha sido creado." << endl;
}


void addUser(const string& a_Username, const string& a_Password, const string& a_Fullname, const string& a_Reposit)
{
    Almacen almacen;
    if (!almacen.exists()) {
        cout << "No existe un almacen que contenga el repositorio '" << a_Reposit << "'" << endl;
        return;
    }
    
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "No existe el repositorio '" << a_Reposit << "'" << endl;
        return;
    }
    
    if (!almacen.addUser(a_Reposit, a_Username, a_Password, a_Fullname)) {
        cout << "El usuario '" << a_Username << "' no ha sido creado." << endl
             << "Verifique que el usuario no pertenezca ya a dicho repositorio." << endl;
        return;
    }

    cout << "El usuario " << a_Username << " ha sido creado exitosamente." << endl;
}


bool fileExists(const string& a_Filename)
{
    std::ifstream file(a_Filename.c_str());
    return file.good();
}


void removeRepository(const string& a_Reposit)
{
    Almacen almacen;
    if (!almacen.exists()) {
        cout << "No existe un almacen que contenga el repositorio '" << a_Reposit << "'" << endl;
        return;
    }
    
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "No existe el repositorio '" << a_Reposit << "'" << endl;
        return;
    }

    if (almacen.removeRepository(a_Reposit))
        cout << "El repositorio '" << a_Reposit << "' ha sido eliminado exitosamente." << endl;
    else
        cout << "El repositorio '" << a_Reposit << "' no ha sido eliminado." << endl;
}


void removeUser(const string& a_Reposit, const string& a_Username)
{
    Almacen almacen;
    if (!almacen.exists()) {
        cout << "No existe un almacen que contenga el repositorio '" << a_Reposit << "'" << endl;
        return;
    }
    
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "No existe el repositorio '" << a_Reposit << "'" << endl;
        return;
    }
    
    if (almacen.removeUser(a_Reposit, a_Username))
        cout << "El usuario " << a_Username << " ha sido eliminado del repositorio '" << a_Reposit << "' exitosamente." << endl;
    else
        cout << "El usuario " << a_Username << " ha sido eliminado del repositorio '" << a_Reposit << "'." << endl;
}


void showHelp(const char* progname)
{
    cout << "uso: " << progname << " [[-a \"nombre repositorio\"] | [-c \"nombre repositorio\"] |" << endl
         << "                 [-e usuario \"nombre repositorio\"] | [-h] | [-o \"nombre repositorio\"] |" << endl
         << "                 [-r \"nombre repositorio\"] |" << endl
         << "                 [-u usuario password \"nombre usuario\" \"nombre repositorio\"]]" << endl
         << "-a, crear almacen de repositorios." << endl
         << "-c, crear repositorio." << endl
         << "-e, eliminar usuario." << endl
         << "-o, obtener listado de usuarios." << endl
         << "-r, eliminar repositorio." << endl
         << "-u, agregar usuario." << endl
         << endl
         << "-h, mostrar esta ayuda." << endl
         ;
}


void showUsers(const string& a_Reposit)
{
    Almacen almacen;
    if (!almacen.exists()) {
        cout << "No existe un almacen que contenga el repositorio '" << a_Reposit << "'" << endl;
        return;
    }
    
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "No existe el repositorio '" << a_Reposit << "'" << endl;
        return;
    }

    std::list<User> users = almacen.getListOfUsers(a_Reposit);
    cout << "Lista de usuarios del repositorio '" << a_Reposit << "':" << endl
         << "[usuario] [nombre completo]" << endl;
    for (std::list<User>::iterator it = users.begin(); it != users.end(); ++it) {
        cout << it->username << " '" << it->fullname << "'" << endl;
    }
}
