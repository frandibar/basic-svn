//filedirbplustree.h

#ifndef FILEDIRBPLUSTREE_H_INCLUDED
#define FILEDIRBPLUSTREE_H_INCLUDED

#include "bplustree.h"
#include "FileDirNode.h"
#include "FileDirIndexNode.h"
#include "FileDirLeafNode.h"
#include <fstream>
#include <string>

using std::string;

class FileDirBPlusTree : public BPlusTree
{
public:
   FileDirBPlusTree();
   virtual ~FileDirBPlusTree();
      
   bool open(const string& a_Filename);
   bool close();

   int search(const char* key);
   int searchFileAndVersion(const char* fileName, int version);
   int getFirstBlock(const char* key);
   
   bool insert(const char* key, int reference);

   void list();

protected:
   
   bool readHeader();
   bool writeHeader();
   
   bool readRoot();
   bool writeRoot();
   
   bool readNode(int id,Node** node);
   bool writeNode(Node* node);
   
   int searchPlace(const char* key);
   
   int searchPlaceRec(const char* key);
   
   void insertarEnPadre(int NroNodoPadre, int NroNodoHijo,const char* claveAlPadre);
   
   bool actualizarPadre(FileDirIndexNode* padre);

   bool isEmpty() const { return _raiz == 0; }

private:
   FileDirNode*   _raiz;
   FileDirNode*   _nodoActual;          
};
#endif
