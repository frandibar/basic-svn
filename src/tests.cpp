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
	FileVersion* version = new FileVersion(1,1,*Timetm,Rodrigo.c_str(),0,'t',FileVersion::t_versionType::MODIFICACION);

	bloque->insertVersion(version);

	delete version;

	time(&Time);
	Timetm = localtime(&Time);
	version = new FileVersion(2,1,*Timetm,Fran.c_str(),300,'t',FileVersion::t_versionType::MODIFICACION);

	bloque->insertVersion(version);
	
	delete version;

	time(&Time);
	Timetm = localtime(&Time);
	version = new FileVersion(3,1,*Timetm,Fernando.c_str(),5000,'t',FileVersion::t_versionType::MODIFICACION);

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
	
	vf.insertVersion(1,Rodrigo.c_str(),*Timetm,5000,'t',FileVersion::t_versionType::MODIFICACION,&bloqueOriginal);

	int i = 1;
	int nuevoBloque;
	
	FileVersionsFile::t_status result;

	do{
		time(&Time);
		Timetm = localtime(&Time);

		i++;

		result = vf.insertVersion(i,Rodrigo.c_str(),*Timetm,5000 + i * 1500,'t',FileVersion::t_versionType::MODIFICACION,bloqueOriginal,&nuevoBloque);
	}while( (result != FileVersionsFile::t_status::OVERFLOW)&&
			(result != FileVersionsFile::t_status::ERROR) );
	
	int original = vf.getLastOriginalVersionNumber(bloqueOriginal);

	int final = vf.getLastVersionNumber(bloqueOriginal);

	FileVersion* version;

	bool found = vf.searchVersion(&version,i-3,bloqueOriginal);

	if(found){
		delete version;
	}

	vf.close();
	
	FileVersionsFile vf2;

	vf2.open("versionFileTest.bin");

	time(&Time);
	Timetm = localtime(&Time);
	
	int nuevoNuevoBloque = -1;

	vf2.insertVersion(18,Rodrigo.c_str(),*Timetm,5000 + 18*1500,'t',FileVersion::t_versionType::MODIFICACION,1,&nuevoNuevoBloque);

	original = vf2.getLastOriginalVersionNumber(1);

	final = vf2.getLastVersionNumber(1);

	found = vf2.searchVersion(&version,18,0);

	if(found)
		delete version;

	found = vf2.searchVersion(&version,18,1);

	if(found)
		delete version;

	found = vf2.searchVersion(&version,7,0);

	if(found)
		delete version;

	vf2.close();

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

	ofstream fs;
	fs.open("temp.txt",ios::out);

	if(cont2.get(offset,fs))
	{
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

	file->write(nextByte);

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
	
	DirectoryVersion* dv = new DirectoryVersion(1,"Rodrigo",*Timetm,DirectoryVersion::t_versionType::MODIFICACION);

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

	dv->write(nextByte);

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
}

void testDirectoryBlock()
{
	time_t Time;
	tm* Timetm;
	
	time(&Time);
	Timetm = localtime(&Time);
	
	DirectoryVersion* dv = new DirectoryVersion(1,"Rodrigo",*Timetm,DirectoryVersion::t_versionType::MODIFICACION);

	dv->addFile("c:\\Rodrigo",1,'t');

	dv->addFile("c:\\Francisco",2,'t');

	dv->addFile("c:\\Fernando",3,'t');

	dv->addFile("c:\\Andres",4,'t');

	DirectoryBlock* db = new DirectoryBlock(0);

	db->insertVersion(dv);

	delete dv;

	time(&Time);
	Timetm = localtime(&Time);
	dv = new DirectoryVersion(2,"Fran",*Timetm,DirectoryVersion::t_versionType::MODIFICACION);

	dv->addFile("c:\\Francisco",5,'t');

	dv->addFile("c:\\Juan",6,'t');

	dv->addFile("c:\\Andres",7,'t');

	db->insertVersion(dv);

	delete dv;

	dv = db->getLastVersion();

	delete dv;

	bool found = db->searchVersion(1);

	found = db->searchVersion(2);

	found = db->searchVersion(5);

	int cantVersiones = db->getCantidadVersiones();

	db->moveFirst();

	while(db->hasNext())
	{
		dv = db->getNext();
		delete dv;
	}

	char* buffer = new char[DirectoryBlock::TAMANIO_BLOQUE_DIRECTORIOS * sizeof(char)];

	db->write(buffer);

	delete db;

	DirectoryBlock* db2 = new DirectoryBlock();

	db2->read(buffer);

	delete buffer;
	delete db2;
}

void testDirectoryVersionsFile()
{
	tm* Timetm;
	time_t Time;

	string Rodrigo = "Rodrigo";
	string Fran = "Francisco";
	string Fernando = "Fernando";	
	
	DirectoryVersionsFile dvf;

	dvf.create("directoryFileTest.bin");

	time(&Time);
	Timetm = localtime(&Time);
	int bloqueOriginal = -1;
	
	DirectoryVersion* dv = new DirectoryVersion(1,Rodrigo.c_str(),*Timetm,DirectoryVersion::t_versionType::MODIFICACION);

	dv->addFile("c:\\Rodrigo.txt",1,'t');
	dv->addFile("c:\\Francisco.bin",1,'b');
	dv->addFile("c:\\Fernando.txt",1,'t');
	dv->addFile("c:\\Luciana.bin",2,'b');

	dvf.insertVersion(dv,&bloqueOriginal);

	delete dv;

	int i = 1;
	int nuevoBloque;
	
	DirectoryVersionsFile::t_status result;

	do{
		time(&Time);
		Timetm = localtime(&Time);

		i++;

		dv = new DirectoryVersion(i,Fernando.c_str(),*Timetm,DirectoryVersion::t_versionType::MODIFICACION);

		dv->addFile("c:\\Rodrigo.txt",i,'t');
		dv->addFile("c:\\Francisco.txt",i,'t');
		dv->addFile("c:\\Fernando.txt",i,'t');
		dv->addFile("c:\\Luciana.txt",i,'t');		

		result = dvf.insertVersion(dv,bloqueOriginal,&nuevoBloque);

		delete dv;
	}while( (result != DirectoryVersionsFile::t_status::OVERFLOW)&&
			(result != DirectoryVersionsFile::t_status::ERROR) );
	
	int final = dvf.getLastVersionNumber(bloqueOriginal);

	DirectoryVersion* version;

	bool found = dvf.searchVersion(&version,i-3,bloqueOriginal);

	if(found){
		delete version;
	}

	found = dvf.getVersion(7,0,&dv);

	if(found)
		delete dv;

	found = dvf.getVersion(3,1,&dv);

	if(found)
		delete dv;

	found = dvf.getVersion(2,0,&dv);

	if(found)
		delete dv;

	dvf.close();
	
	DirectoryVersionsFile dvf2;

	dvf2.open("directoryFileTest.bin");

	time(&Time);
	Timetm = localtime(&Time);
	
	int nuevoNuevoBloque = -1;

	dv = new DirectoryVersion(i,Fernando.c_str(),*Timetm,DirectoryVersion::t_versionType::MODIFICACION);

	dv->addFile("c:\\Rodrigo.txt",i,'t');
	dv->addFile("c:\\Francisco.txt",i,'t');
	dv->addFile("c:\\Fernando.txt",i,'t');
	dv->addFile("c:\\Luciana.txt",i,'t');


	dvf2.insertVersion(dv,nuevoBloque,&nuevoNuevoBloque);

	delete dv;

	final = dvf2.getLastVersionNumber(1);

	found = dvf2.searchVersion(&dv,i,0);

	if(found)
		delete dv;

	found = dvf2.searchVersion(&dv,i,1);

	if(found)
		delete dv;

	found = dvf2.searchVersion(&version,i - 5,0);

	if(found)
		delete dv;

	dvf2.close();
}

