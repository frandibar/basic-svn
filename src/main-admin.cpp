// main-admin.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>      // getenv

#include "Almacen.h"
#include "debug.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;


string CONFIG_FILE;


void addReposit(const string& a_Name);
void addUser(const string& a_Username, const string& a_Pass, const string& a_Fullname, const string& a_Reposit);
void createAlmacen(const string& a_Dir);
bool fileExists(const string& a_Filename);
void removeReposit(const string& a_Name);
void removeUser(const string& a_User, const string& a_Reposit);
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
                if (argsok) addReposit(optarg);
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
                if (argsok) removeReposit(optarg);
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


void addReposit(const string& a_Name)
{
    Almacen almacen;
    if (!almacen.exists()) {
        cout << "No existe un almacen para alojar el repositorio." << endl;
        return;
    }
    
    // agregar repositorio al archivo de configuracion
    if (!almacen.addReposit(a_Name)) {
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
    }

    if (almacen.create(a_Dir))
        cout << "El almacen " << a_Dir << " ha sido creado con exito." << endl;
    else
        cout << "El almacen " << a_Dir << " no ha sido creado." << endl;
}



void addUser(const string& a_Username, const string& a_Pass, const string& a_Fullname, const string& a_Reposit)
{
    // try to open file CONFIG_FILE
    // fail if not exists
    if (!fileExists(CONFIG_FILE)) {
        cout << "No existe un almacen para alojar el repositorio." << endl;
        return;
    }
    
	XMLConfigData config(CONFIG_FILE);
    User u;
    u.username = a_Username;
    u.password = a_Pass;
    u.fullname = a_Fullname;
    if (!config.addUser(a_Reposit, u)) {
        cout << "El usuario " << optarg << " no ha sido creado." << endl
             << "Verifique que el repositorio '" << a_Reposit << "' exista y que dicho usuario no pertenezca ya a dicho repositorio." << endl;
        return;
    }
	config.commit();
    cout << "El usuario " << optarg << " ha sido creado exitosamente." << endl;
}


bool fileExists(const string& a_Filename)
{
    std::ifstream file(a_Filename.c_str());
    return file.good();
}


void removeReposit(const string& a_Name)
{
	XMLConfigData config(CONFIG_FILE);
    if (!config.removeReposit(a_Name)) {
        cout << "El repositorio '" << a_Name << "' no ha sido eliminado." << endl
             << "Verifique que el mismo exista." << endl;
        return;
    }
	config.commit();
    remove(string(config.getDirAlmacen() + "//" + a_Name).c_str());
    cout << "El repositorio '" << a_Name << "' ha sido eliminado exitosamente." << endl;
}


void removeUser(const string& a_Reposit, const string& a_User)
{
	XMLConfigData config(CONFIG_FILE);
    if (!config.removeUser(a_Reposit, a_User)) {
        cout << "El usuario " << a_User << " no ha sido eliminado del repositorio '" << a_Reposit << "'." << endl
             << "Verifique que el mismo exista, al igual que el repositorio." << endl;
        return;
    }
	config.commit();
    cout << "El usuario " << a_User << " ha sido eliminado del repositorio '" << a_Reposit << "' exitosamente." << endl;
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
	XMLConfigData config(CONFIG_FILE);
    if (!config.repositExists(a_Reposit)) {
        cout << "No se ha podido obtener la lista de usuarios del repositorio '" << a_Reposit << "'." << endl
             << "Dicho repositorio no existe." << endl;
        return;
    }
    XMLConfigData::UsersList users = config.getUsersList(a_Reposit);
    cout << "Lista de usuarios del repositorio '" << a_Reposit << "':" << endl
         << "[usuario] [nombre completo]" << endl;
    for (XMLConfigData::UsersList::iterator it = users.begin(); it != users.end(); ++it) {
        cout << it->username << " '" << it->fullname << "'" << endl;
    }
}
