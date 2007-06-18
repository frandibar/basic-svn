#include "DirectoryVersion.h"

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#define SIZE  1024
#define SIZE2 1004

using namespace std;

int main(int argc, char** argv)
{
   // usage: ./directoryVersionsReader filename
    if (argc != 2)
        return -1;

    ifstream is(argv[1]);
    if (!is.is_open()) return false;

    char buffer[SIZE];
    is.read(buffer, SIZE);
    int nBloques;
    memcpy(&nBloques, buffer, sizeof(int));
    cout << "#bloques  : (" << sizeof(int) << "b 0)\t" << nBloques << endl;
    cout << "-------------------------------------" << endl;

    for (int i = 0; i < nBloques && !is.eof(); ++i) {

        int pad = SIZE * (i + 1);

        is.read(buffer, SIZE);
        char* offset = buffer;

        int numero;
        memcpy(&numero, offset, sizeof(int));
        offset += sizeof(int);
        cout << "numero   : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << numero << endl;

        int anterior;
        memcpy(&anterior, offset, sizeof(int));
        offset += sizeof(int);
        cout << "anterior      : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << anterior << endl;

        int siguiente;
        memcpy(&siguiente, offset, sizeof(int));
        offset += sizeof(int);
        cout << "siguiente   : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << siguiente << endl;

        int espacioLibre;
        memcpy(&espacioLibre, offset, sizeof(int));
        offset += sizeof(int);
        cout << "e libre : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << espacioLibre << endl;

        int nVersiones;
        memcpy(&nVersiones, offset, sizeof(int));
        offset += sizeof(int);
        cout << "#versiones : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << nVersiones << endl;

        cout << "-------------------------------------" << endl;

		  int bytesLeidosEnData = 0;
		  for(int i = 0;i < nVersiones;i++)
		  {
	        int versionNumber;
	        memcpy(&versionNumber, offset, sizeof(int));
	        offset += sizeof(int);
			  bytesLeidosEnData += sizeof(int);
	        cout << "version number  : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << versionNumber << endl;

			  int userlength;
			  memcpy(&userlength, offset, sizeof(int));
			  offset += sizeof(int);
			  bytesLeidosEnData += sizeof(int);
	        cout << "userlength : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << userlength << endl;
			
			  char* user = new char[(userlength + 1) * sizeof(char)];
			  memcpy(user,offset,userlength * sizeof(char));
			  user[userlength] = 0;
			  offset += sizeof(char) * userlength;
			  bytesLeidosEnData += userlength * sizeof(char);
	        cout << "user : (" << userlength * sizeof(char) << "b " << offset - buffer + pad << ")\t" << user << endl;

			  delete user;

			  tm Time;
			  memcpy(&Time, offset,sizeof(tm));
			  offset += sizeof(tm);			  
			  bytesLeidosEnData += sizeof(tm);
	        cout << "Fecha y Hora: (" << sizeof(tm) << "b " << offset - buffer + pad << ")\t" << asctime(&Time);

			  DirectoryVersion::t_versionType tipoVersion;
			  memcpy(&tipoVersion, offset, sizeof(DirectoryVersion::t_versionType));
			  offset += sizeof(DirectoryVersion::t_versionType);
			  bytesLeidosEnData += sizeof(DirectoryVersion::t_versionType);
	        cout << "tipo de version : (" << sizeof(DirectoryVersion::t_versionType) << "b " << offset - buffer + pad << ")\t" << tipoVersion 			  << endl;

			  int listSize;
			  memcpy(&listSize, offset, sizeof(int));
			  offset += sizeof(int);
			  bytesLeidosEnData += sizeof(int);
	        cout << "file offset : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << listSize << endl;
				
			  for(int i = 0;i < listSize;i++)
			  {
					int namelength;
					memcpy(&namelength, offset, sizeof(int));
					offset += sizeof(int);
					bytesLeidosEnData += sizeof(int);
	        		cout << "namelength : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << namelength << endl;

				  	char* name = new char[(namelength + 1) * sizeof(char)];
				  	memcpy(name,offset,namelength * sizeof(char));
				  	name[namelength] = 0;
				  	offset += sizeof(char) * namelength;
				  	bytesLeidosEnData += namelength * sizeof(char);
		        	cout << "name : (" << namelength * sizeof(char) << "b " << offset - buffer + pad << ")\t" << name << endl;

					delete name;

					int fileversionnumber;
					memcpy(&fileversionnumber,offset,sizeof(int));
					offset += sizeof(int);
					bytesLeidosEnData += sizeof(int);
		        	cout << "file version number : (" << sizeof(int) << "b " << offset - buffer + pad << ")\t" << fileversionnumber << 						endl;

					char filetype;
					memcpy(&filetype, offset, sizeof(char));
					offset += sizeof(char);
					bytesLeidosEnData += sizeof(char);
		        	cout << "file type : (" << sizeof(char) << "b " << offset - buffer + pad << ")\t" << filetype << 										 						endl;
        	  
					cout << "_____________________________________" << endl;
										
      	  }
			 	
        	  cout << "-------------------------------------" << endl;
			  cout << "-------------------------------------" << endl;
		  }

		  char* data = new char[(SIZE2 - bytesLeidosEnData + 4) * sizeof(char)];
	     memcpy(data, offset,(SIZE2 - bytesLeidosEnData + 4) * sizeof(char));
	     offset += (SIZE2 - bytesLeidosEnData + 4) * sizeof(char);
	     cout << "data    : (" << (SIZE2 - bytesLeidosEnData) * sizeof(char) << "b " << offset - buffer + pad << ")\t" << data << endl;
		  delete data;

        cout << "-------------------------------------" << endl;
    }
    return 0;
}


