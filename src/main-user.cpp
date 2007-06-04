// main-usr.cpp

#include "Almacen.h"
#include "debug.h"

#include <iostream>
#include <string>
#include <fstream>
#include <exception>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

void add        (const string& user, const string& pass, const string& repository, const string& file);
void showHistory(const string& user, const string& pass, const string& repository, const string& file);
void getFile    (const string& user, const string& pass, const string& repository, const string& dir, const string& file, const string& version);
bool validateUser(const string& a_Repository, const string& a_Username, const string& a_Password);
void showHelp(const char* progname);
void showDiff(const string& user, const string& pass, const string& reposit, const string& versionA, const string& versionB, const string& filename);
void showByDate(const string& user, const string& pass, const string& reposit, const string& date);


int main(int argc, char** argv)
{
    int c; 
    string user, pass;
    bool argsok = false;
    while ((c = getopt(argc, argv, "-a:d:f:hl:o:")) != -1) {
        switch (c) {

            case 1: // non-option ARGV-elements
                if (user.length() == 0) 
                    user = optarg;
                else if (pass.length() == 0) 
                    pass = optarg;
                // else invalid argument
                break;

            case 'a': // agregar archivo o cambios al repositorio
                // -a "nombre repositorio" "nombre archivo/directorio"
                argsok = (argc == 6);
                if (argsok) add(user, pass, optarg, argv[optind]);
                break;

            case 'd': // mostrar diff entre 2 versiones
                // -d "nombre repositorio" version_inicial version_final ["nombre archivo/directorio"]
                argsok = ((argc == 6) || (argc == 7));
                if (argsok) {
                    if (argc == 6)
                        showDiff(user, pass, optarg, argv[optind], argv[optind + 1], "");
                    else
                        showDiff(user, pass, optarg, argv[optind], argv[optind + 1], argv[optind + 2]);
                }
                break;

            case 'f': // mostrar cambios en determinada fecha
                // -f usuario "nombre repositorio" fecha
                argsok = (argc == 7);
                if (argsok) showByDate(user, pass, optarg, argv[optind]);
                break;

            case 'h':
                showHelp(argv[0]);
                argsok = true;
                break;

            case 'l': // cambios a un archivo
                // -l "nombre repositorio" "nombre archivo/dir"
                argsok = (argc == 6);
                if (argsok) showHistory(user, pass, optarg, argv[optind]);
                break;

            case 'o': // obtener archivo
                // -o "nombre repositorio" "nombre directorio destino" "nombre archivo/dir" [version]
                argsok = (argc == 8);
                if (argsok) getFile(user, pass, optarg, argv[optind], argv[optind + 1], argv[optind + 2]);
                break;
        }
    }

    if (!argsok) {
       cout << "Parametros invalidos." << endl;
       showHelp(argv[0]);
    }
   
}

void add(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_Filename)
{
    Almacen almacen;
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "El repositorio " << a_Reposit << " no existe." << endl;
        return;
    }
    if (!validateUser(a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.addFile(a_Reposit, a_Filename, a_Username, a_Password)) {
        cout << "El archivo " << a_Filename << " no pudo ser agregado." << endl;
        return;
    }

    cout << "El archivo " << a_Filename << " ha sido agregado exitosamente." << endl;
}


void showHistory(const string& user, const string& pass, const string& reposit, const string& file)
{
    cout << "showHistory" << user << " " << pass << " " << reposit << " " << file << endl;    
}

bool validateUser(const string& a_Reposit, const string& a_Username, const string& a_Password)
{
    Almacen almacen;
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "El repositorio " << a_Reposit << " no existe." << endl;
        return false;
    }

    if (!almacen.userExists(a_Reposit, a_Username)) {
        cout << "El usuario " << a_Username << " no pertenece al repositorio " << a_Reposit << "." << endl;
        return false;
    }

    if (!almacen.validatePassword(a_Reposit, a_Username, a_Password)) {
        cout << "Contraseña invalida." << endl;
        return false;
    }

    return true;

}


void showDiff(const string& user, const string& pass, const string& reposit, const string& versionA, const string& versionB, const string& filename = "")
{
    cout << "showDiff " << user << " " << pass << " " << reposit << " " << versionA << " " << versionB << endl;
}

void showByDate(const string& user, const string& pass, const string& reposit, const string& date)
{
    cout << "showByDate " << user << " " << pass << " " << reposit << " " << date << endl;
}

void getFile(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_TargetDir, const string& a_Filename, const string& a_Version)
{
    Almacen almacen;
    if (!almacen.repositoryExists(a_Reposit)) {
        cout << "El repositorio " << a_Reposit << " no existe." << endl;
        return;
    }
    if (!validateUser(a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.getFile(a_Reposit, a_TargetDir, a_Filename, a_Version, a_Username, a_Password)) {
        cout << "El archivo " << a_Filename << " no pudo ser recuperado." << endl;
        return;
    }
    cout << "El archivo " << a_Filename << " ha sido recuperado exitosamente." << endl;
}

void showHelp(const char* progname)
{
    cout << "uso: " << progname << " usuario contraseña [[-a \"nombre repositorio\" \"nombre archivo/directorio\" |" << endl
         << "                 [-d \"nombre repositorio\" version_inicial version_final] |" << endl
         << "                 [-f usuario \"nombre repositorio\" fecha(dd/mm/aa)] | [-h] |" << endl
         << "                 [-l \"nombre repositorio\" \"nombre archivo/directorio\"] |" << endl
         << "                 [-o \"nombre repositorio\" \"nombre directorio destino\" \"nombre archivo/directorio\" [version]]" << endl
         << "-a, almacenar archivos y directorios." << endl
         << "-d, ver diferencias entre dos versiones." << endl
         << "-f, ver las actualizaciones en una fecha dada." << endl
         << "-l, ver historial de cambios a un archivo o directorio." << endl
         << "-o, obtener una determinada version de un archivo o directorio." << endl
         << endl
         << "-h, mostrar esta ayuda." << endl
         ;
}

