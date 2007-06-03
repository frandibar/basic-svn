#include "tests.h"


using std::string;
using std::list;
using std::ifstream;

using namespace std;

void testBloque()
{
	tm* Timetm;
	time_t Time;

	string Rodrigo = "Rodrigo";
	string Fran = "Francisco";
	string Fernando = "Fernando";

	FileBlock* bloque = new FileBlock(0,-1,-1);
	
	time(&Time);
	Timetm = localtime(&Time);
    FileVersion* version = new FileVersion(1,1,*Timetm,Rodrigo.c_str(),0,'t');

	bloque->insertVersion(version);

	delete version;

	time(&Time);
	Timetm = localtime(&Time);
	version = new FileVersion(2,1,*Timetm,Fran.c_str(),300,'t');

	bloque->insertVersion(version);
	
	delete version;

	time(&Time);
	Timetm = localtime(&Time);
	version = new FileVersion(3,1,*Timetm,Fernando.c_str(),5000,'t');

	bloque->insertVersion(version);
	
	delete version;

	bool found = bloque->searchVersion(2);
	found = bloque->searchVersion(1);
	found = bloque->searchVersion(3);
	found = bloque->searchVersion(10);

	found = bloque->searchVersion(1,&version);
	delete version;
	
	found = bloque->searchVersion(3,&version);
	delete version;
	
	found = bloque->searchVersion(10,&version);	

	found = bloque->searchVersion(2,&version);
	delete version;

	version = bloque->getLastVersion();
	delete version;

	bloque->moveFirst();
	while(bloque->hasNext())
	{
		version = bloque->getNext();
		delete version;
	}

	int firstVersion = bloque->getFirstVersionNumber();

	int sig = bloque->getSiguiente();

	int ant = bloque->getAnterior();

	int nro = bloque->getNumero();

	int cantVersiones = bloque->getCantidadVersiones();

	char* buffer = new char[FileBlock::TAMANIO_BLOQUE_ARCHIVOS * sizeof(char)];

	bloque->write(buffer);

	delete bloque;

	FileBlock* bloque2 = new FileBlock();

	bloque2->read(buffer);

	delete bloque2;

	delete buffer;
}

void testVersionFile()
{
	tm* Timetm;
	time_t Time;

	string Rodrigo = "Rodrigo";
	string Fran = "Francisco";
	string Fernando = "Fernando";	
	
	FileVersionsFile vf;

	vf.create("versionFileTest.bin");

	time(&Time);
	Timetm = localtime(&Time);
	int bloqueOriginal = -1;
	
	vf.insertVersion(1,Rodrigo.c_str(),*Timetm,5000,'t',&bloqueOriginal);

	int i = 1;
	int nuevoBloque;
	
	do{
		time(&Time);
		Timetm = localtime(&Time);

		i++;				
	}while(vf.insertVersion(i,Rodrigo.c_str(),*Timetm,5000 + i * 1500,'t',bloqueOriginal,&nuevoBloque) != FileVersionsFile::t_status::OVERFLOW);
	
	int original = vf.getLastOriginalVersionNumber(bloqueOriginal);

	int final = vf.getLastVersionNumber(bloqueOriginal);

	FileVersion* version;

	bool found = vf.searchVersion(&version,i-3,bloqueOriginal);

	if(found){
		delete version;
	}

	vf.close();
	
/*	VersionFile vf2;

	vf2.open("versionFileTest.bin");

	time(&Time);
	Timetm = localtime(&Time);
	
	int nuevoNuevoBloque = -1;

	vf2.insertVersion(18,Rodrigo.c_str(),*Timetm,5000 + 18*1500,'t',1,&nuevoNuevoBloque);

	int original = vf2.getLastOriginalVersionNumber(1);

	int final = vf2.getLastVersionNumber(1);

	Version* version;
	bool found = vf2.searchVersion(&version,18,0);

	if(found)
		delete version;

	found = vf2.searchVersion(&version,18,1);

	if(found)
		delete version;

	found = vf2.searchVersion(&version,7,0);

	if(found)
		delete version;

	vf2.close();*/

	FileVersionsFile vf3;

	vf3.open("versionFileTest.bin");

	list<FileVersion> lstVersions;

	bool ok = vf3.getVersionFrom(1,10,1,lstVersions);
	
	list<FileVersion>::iterator it;

	FileVersion aux;

	for(it = lstVersions.begin();it != lstVersions.end();it++)
	{
		int nro_version = (it)->getNroVersion();
	}

	lstVersions.erase(lstVersions.begin(),lstVersions.end());

	ok = vf3.getVersionFrom(5,15,1,lstVersions);

	for(it = lstVersions.begin();it != lstVersions.end();it++)
	{
		int nro_version = (it)->getNroVersion();
	}

	ok = vf3.getVersionFrom(5,5,1,lstVersions);

	ok = vf3.getVersionFrom(10,18,1,lstVersions);

	ok = vf3.getVersionFrom(5,8,1,lstVersions);

	vf3.close();
}

void buildList(list<int>& lst,int fin)
{
	for(int i = 1; i < fin; i++)
		lst.push_back(i);

	return;
}

void testList()
{
	list<int> lst;

	buildList(lst,10);

	list<int>::iterator it;

	for(it = lst.begin();it != lst.end();it++)
	{
		int nro = *it;
	}

	lst.erase(lst.begin(),lst.end());

	buildList(lst,15);

	lst.clear();
}

void testContainer()
{

	Container cont;

	cont.create("containerTest.bin");

	cont.close();

	cont.open("containerTest.bin");

	ifstream is("helpers.cpp");
	
	int offset = cont.append(is);

	is.close();

	is.open("Version.h");

	int offset2 = cont.append(is);

	is.close();

	is.open("Bloque.cpp");

	int offset3 = cont.append(is);

	is.close();

	cont.close();

	Container cont2;

	cont2.open("containerTest.bin");

	fstream fs;

	fs.open("temp.txt",ios::out);

	if(cont2.get(offset,fs))
	{
		fs.seekg(0,ios::end);
		fs.seekp(0,ios::end);

		int tamanio = fs.tellp();

	}

	else fs.close();
}

void testFile()
{
	File* file = new File("c:\\Rodrigo",1,'t');

	char* nombre = file->getName();

	int version = file->getVersion();

	char tipo = file->getType();

	int tamanio = file->getTamanioEnDisco();

	char* buffer = (char*) malloc((2*tamanio) * sizeof(char));

	char* nextByte = buffer;

	file->write(&nextByte);

	delete file;

	File* file2 = new File();

	nextByte = buffer;

	file2->read(&nextByte);

	delete file2;
	delete buffer;
}

void testDirectoryVersion()
{
	time_t Time;
	tm* Timetm;
	
	time(&Time);
	Timetm = localtime(&Time);
	
	DirectoryVersion* dv = new DirectoryVersion(1,"Rodrigo",*Timetm);

	int files = dv->getCantFile();

	int nroVersion = dv->getNroVersion();

	*Timetm = dv->getDate();

	char* usuario = dv->getUser();

	dv->addFile("c:\\Rodrigo",1,'t');

	dv->addFile("c:\\Francisco",2,'t');

	dv->addFile("c:\\Fernando",3,'t');

	dv->addFile("c:\\Andres",4,'t');

	files = dv->getCantFile();

	int tamanio = dv->tamanioEnDisco();

	char* buffer = (char*) malloc(sizeof(char) * (tamanio * 2));

	char * nextByte = buffer;

	dv->write(&nextByte);

	delete dv;

	DirectoryVersion* dv2 = new DirectoryVersion();

	nextByte = buffer;
	
	dv2->read(&nextByte);

	list<File>* fl = dv2->getFilesList();

	list<File>::iterator iterador;

	for(iterador = fl->begin();iterador != fl->end();iterador++)
	{
		char* nombre = iterador->getName();

		int version = iterador->getVersion();

		char tipo = iterador->getType();
	}

	delete dv2;

	//fl->clear();
}
