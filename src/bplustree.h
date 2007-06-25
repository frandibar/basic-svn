//bplustree.h

#ifndef BPLUSTREE_H_INCLUDED
#define BPLUSTREE_H_INCLUDED

#include <fstream>
#include <string>

#include "Node.h"

using std::string;

class BPlusTree
{
public:
   BPlusTree();
   virtual ~BPlusTree();

   bool create(const string& a_Filename);
   bool destroy();

   virtual bool open(const string& a_Filename) = 0;
   virtual bool close() = 0;

   virtual int search(const char* key) = 0;
   virtual bool insert(const char* key, int reference) = 0;

protected:
   virtual bool readHeader() = 0;
   virtual bool writeHeader() = 0;

   virtual bool readRoot() = 0;
   virtual bool writeRoot() = 0;

   virtual bool readNode(int id, Node** node) = 0;
   virtual bool writeNode(Node* node) = 0;
   
   virtual int searchPlace(const char* key) = 0;
   
   virtual int searchPlaceRec(const char* key) = 0;

   virtual void insertarEnPadre(int NroNodoPadre, int NroNodoHijo, const char* claveAlPadre) = 0;
   
   virtual bool isEmpty() const = 0;

   int            _nNodos;
   char*          _buffer;
   std::fstream   _filestr;
   string         _filename;
   bool           _isOpen;   
};

#endif
