//varlenbplustree.h

#ifndef VARLENBPLUSTREE_H_INCLUDED
#define VARLENBPLUSTREE_H_INCLUDED

#include "bplustree.h"
#include "VarLenNode.h"
#include "VarLenIndexNode.h"
#include "VarLenLeafNode.h"
#include <fstream>
#include <string>

using std::string;

class VarLenBPlusTree : public BPlusTree
{
public:
   VarLenBPlusTree();
   virtual ~VarLenBPlusTree();

   bool open(const string& a_Filename);
   bool close();

   bool insert(const char* key, int reference);

   int search(const char* key);

protected:
   bool readHeader();
   bool writeHeader();

   bool readRoot();
   bool writeRoot();

   bool readNode(int id, Node** node);
   bool writeNode(Node* node);

   int searchPlace(const char* key);
   
   int searchPlaceRec(const char* key);

   void insertarEnPadre(int NroNodoPadre, int NroNodoHijo,const char* claveAlPadre);
   
   bool actualizarPadre(VarLenIndexNode* padre);
   
   bool isEmpty() const { return _raiz == 0; }

private:
   VarLenNode* _raiz;
   VarLenNode* _nodoActual;
};


#endif
