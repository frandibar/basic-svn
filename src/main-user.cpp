
#include "Almacen.h"
#include "debug.h"
#include "helpers.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

bool validateUserAndRepository(Almacen* a_Almacen, const string& a_Reposit, const string& a_Username, const string& a_Password)
{
    if (!a_Almacen->repositoryExists(a_Reposit)) {
        cout << "El repositorio " << a_Reposit << " no existe." << endl;
        return false;
    }

    if (!a_Almacen->userExists(a_Reposit, a_Username)) {
        cout << "El usuario " << a_Username << " no pertenece al repositorio " << a_Reposit << "." << endl;
        return false;
    }

    if (!a_Almacen->validatePassword(a_Reposit, a_Username, a_Password)) {
        cout << "Contraseņa invalida." << endl;
        return false;
    }

    return true;
}


void add(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_Target)
{
    Almacen almacen;
    if (!validateUserAndRepository(&almacen, a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.add(a_Reposit, a_Target, a_Username, a_Password)) {
        cout << "El archivo " << a_Target << " no pudo ser agregado." << endl;
        return;
    }

    cout << "El archivo " << a_Target << " ha sido agregado exitosamente." << endl;
}


void showHistory(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_Filename = "")
{
    Almacen almacen;
    if (!validateUserAndRepository(&almacen, a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.getHistory(cout, a_Username, a_Password, a_Reposit, a_Filename))
        cout << "No se pudo obtener la historia del archivo solicitado." << endl;
}


void showDiff(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_VersionA, const string& a_VersionB, const string& a_Filename = "")
{
    Almacen almacen;
    if (!validateUserAndRepository(&almacen, a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.getDiff(cout, a_Username, a_Password, a_Reposit, a_VersionA, a_VersionB, a_Filename)) {
        cout << "No se pudieron obtener las versiones solicitadas." << endl;
        return;
    }
}


void showByDate(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_Date)
{
    Almacen almacen;
    if (!validateUserAndRepository(&almacen, a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.getDiffByDate(cout, a_Username, a_Password, a_Reposit, a_Date))
        cout << "No se pudieron obtener las versiones solicitadas." << endl;
}

void changePassword(const string& a_Username, const string& a_Password, const string& a_NewPassword, const string& a_Reposit)
{
    Almacen almacen;
    if (!validateUserAndRepository(&almacen, a_Reposit, a_Username, a_Password))
        return;
    
    if (almacen.changePassword(a_Reposit, a_Username, a_NewPassword))
        cout << "La  contraseņa ha sido actualizada exitosamente." << endl;
    else
        cout << "La  contraseņa no ha podido ser actualizada." << endl;


}

void get(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_TargetDir, const string& a_Filename, const string& a_Version)
{
    Almacen almacen;
    if (!validateUserAndRepository(&almacen, a_Reposit, a_Username, a_Password))
        return;

    if (!almacen.get(a_Reposit, a_TargetDir, a_Filename, a_Version, a_Username, a_Password)) {
        cout << "El archivo " << a_Filename << " no pudo ser recuperado." << endl;
        return;
    }
    cout << "El archivo " << a_Filename << " ha sido recuperado exitosamente." << endl;
}


void showChanges(const string& a_Username, const string& a_Password, const string& a_Reposit, const string& a_Num = "")
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

    cout << "Lista de ultimas modificaciones al repositorio '" << a_Reposit << "':" << endl;
    if (!almacen.getListOfChanges(cout, a_Reposit, a_Username, a_Password, fromString<int>(a_Num), false)) {
        cout << "No se ha podido obtener el listado, verifique que el usuario y contraseņa sean validos." << endl;
        return;
    }
}


void showHelp(const char* progname)
{
    cout << "uso: " << progname << " usuario contraseņa [[-a \"nombre repositorio\" \"nombre archivo/directorio\"] |" << endl
         << "                 [-d \"nombre repositorio\" version_inicial version_final] [\"nombre archivo/directorio\"]] |" << endl
         << "                 [-f \"nombre repositorio\" fecha(aaaa/mm/dd)]] | [-h] |" << endl
         << "                 [-l \"nombre repositorio\" [\"nombre archivo/directorio\"]] |" << endl
         << "                 [-m \"nombre repositorio\" [cantidad]] |" << endl
         << "                 [-o \"nombre repositorio\" \"nombre directorio destino\" \"nombre archivo/directorio\" [version]] |" << endl
         << "                 [-p nueva \"nombre repositorio\" ]]" << endl
         << "-a, almacenar archivos y directorios." << endl
         << "-d, ver diferencias entre dos versiones." << endl
         << "-f, ver las actualizaciones en una fecha dada." << endl
         << "-l, ver historial de cambios a un archivo o directorio." << endl
         << "-m, obtener listado de ultimos cambios efectuados por un usuario." << endl
         << "-o, obtener una determinada version de un archivo o directorio." << endl
         << "-p, cambiar contraseņa." << endl
         << endl
         << "-h, mostrar esta ayuda." << endl
         ;
}


int main(int argc, char** argv)
{
    int c; 
    string user, pass;
    bool argsok = false;
    while ((c = getopt(argc, argv, "-a:d:f:hl:m:o:p:")) != -1) {
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
                argsok = ((argc == 7) || (argc == 8));
                if (argsok) {
                    if (argc == 7)
                        showDiff(user, pass, optarg, argv[optind], argv[optind + 1], "");
                    else
                        showDiff(user, pass, optarg, argv[optind], argv[optind + 1], argv[optind + 2]);
                }
                break;

            case 'f': // mostrar cambios en determinada fecha
                // -f usuario "nombre repositorio" fecha
                argsok = (argc == 6);
                if (argsok) showByDate(user, pass, optarg, argv[optind]);
                break;

            case 'h':
                showHelp(argv[0]);
                argsok = true;
                break;

            case 'l': // cambios a un archivo
                // -l "nombre repositorio" "nombre archivo/dir"
                argsok = ((argc == 5) || (argc == 6));
                if (argsok) 
                    if (argc == 5)
                        showHistory(user, pass, optarg, "");
                    else
                        showHistory(user, pass, optarg, argv[optind]);
                break;

            case 'm': // listar cambios de usuario
                // -m rep [cant]
                argsok = ((argc == 5) || (argc == 6));
                if (argsok) { 
                    if (argc == 5)
                        showChanges(user, pass, optarg);
                    else
                        showChanges(user, pass, optarg, argv[optind]);
                }
                break;

            case 'o': // obtener archivo
                // -o "nombre repositorio" "nombre directorio destino" ["nombre archivo/dir"] [version]
                argsok = ((argc == 6) || (argc == 7) || (argc == 8));
                if (argsok) {
                    if (argc == 6)
                        get(user, pass, optarg, argv[optind], "", ""); 
                    else if (argc == 7)
                        get(user, pass, optarg, argv[optind], argv[optind + 1], "");
                    else
                        get(user, pass, optarg, argv[optind], argv[optind + 1], argv[optind + 2]);
                }
                break;

            case 'p': // cambiar password
                // -p nueva "nombre repositorio"
                argsok = (argc == 6);
                if (argsok) changePassword(user, pass, optarg, argv[optind]);
                break;
        }
    }

    if (!argsok) {
       cout << "Parametros invalidos." << endl;
       showHelp(argv[0]);
    }
   
}

