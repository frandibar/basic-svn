//varlenbplustree.cpp

#include "VarLenBPlusTree.h"
#include "debug.h"

using std::ios;

VarLenBPlusTree::VarLenBPlusTree() : BPlusTree()
{
   _raiz = 0;
   _nodoActual = 0;
   _buffer = new char[VarLenNode::VARLEN_NODE_SIZE];
}

VarLenBPlusTree::~VarLenBPlusTree()
{
   delete _buffer;
   delete _raiz;
   delete _nodoActual;
}

bool VarLenBPlusTree::open(const string& a_Filename)
{
    if (_isOpen) 
        return false;

    debug("opening bplustree '" + a_Filename + "'\n");
    _filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
    _isOpen = _filestr.is_open() && readHeader();

    if (_nNodos > 0)
        _isOpen = _isOpen && readRoot();

    if (_isOpen) {
        _nodoActual = _raiz;
        _filename = a_Filename;
    }
    debug("bplustree open " + string(_isOpen ? "successfull" : "failed") + "\n");
    return _isOpen;
}

bool VarLenBPlusTree::close()
{
    if (!_isOpen)
        return true;

    bool ret = true;
    debug("closing bplustree " + _filename + "\n");
    if (writeHeader()) {
        if((_raiz != 0)  && writeRoot())
         if (_raiz != _nodoActual)
            ret = writeNode(_nodoActual);
    }
    else 
        ret = false;

    _filestr.close();
    _isOpen = _filestr.is_open();
    debug("bplustree close " + string((!_isOpen && ret) ? "successfull" : "failed") + "\n");
    return (!_isOpen && ret);
}

bool VarLenBPlusTree::insert(const char* key, int reference)
{
    VarLenLeafNode* hojaNueva;
    VarLenLeafNode* hnoDerecho;
	int bytesArreglo = 0;
	int clavesArreglo = 0;
	char* arreglo;
	char* claveARaiz;

    if (_nNodos > 0) {
        int nodoReceptor = searchPlace(key);
        if (nodoReceptor) {              // voy a insertar en un nodo != _raiz
            char* claveAlPadre;
            switch (_nodoActual->insert(key, reference, &clavesArreglo, &arreglo, &bytesArreglo)) {
            case 1:
                return true;

            case 2:                 
                int nroNuevoNodo;
                    
                // creo una nueva hoja haciendo el split
                hojaNueva = static_cast<VarLenLeafNode*>((static_cast<VarLenLeafNode*> (_nodoActual))->split
									(_nNodos,arreglo,bytesArreglo,clavesArreglo,&claveAlPadre));

				// libero el arreglo
				delete(arreglo);

                // tengo que levantar el hno derecho de la nueva hoja y setearle la nueva hoja 
                // como su hno izquierdo
                if (hojaNueva->getHnoDerecho() > 0) {
                    hnoDerecho = 0;
                    readNode(hojaNueva->getHnoDerecho(), (Node**)&hnoDerecho);
                    hnoDerecho->setHnoIzquierdo(hojaNueva->getId());
                    writeNode(hnoDerecho);
                    delete hnoDerecho;
                }
                    
                // incremento la cantidad de nodos
                _nNodos++;
                    
                nroNuevoNodo = hojaNueva->getId();

                // escribo los dos nodos
                writeNode(_nodoActual);
                writeNode(hojaNueva);

                delete hojaNueva;
                insertarEnPadre(_nodoActual->getPadre(), nroNuevoNodo, claveAlPadre);
				delete(claveAlPadre);
                return true;

            default:
                return false;
            }
        }
        else {
            VarLenIndexNode* nuevaRaiz;
            int idH1;
            int idH2;
            VarLenLeafNode* nuevaHoja;
            VarLenLeafNode* nuevaHoja2;
            switch (_raiz->insert(key,reference, &clavesArreglo, &arreglo, &bytesArreglo)) {
            case 1:
                return true;

            case 2:
                nuevaHoja = 0;
                nuevaHoja2 = 0;
                
                // creo 2 nodos hoja nuevos para hacer el split y hacer que la nueva raiz sea indice
                _raiz->promoteRoot( (VarLenNode**)(&nuevaHoja),
                                    (VarLenNode**)(&nuevaHoja2),
                                    _nNodos, _nNodos + 1,
									clavesArreglo, &arreglo, bytesArreglo, &claveARaiz);

                _nNodos += 2;

                // escribo las 2 nuevas hojas
                writeNode(nuevaHoja);
                writeNode(nuevaHoja2);

                idH1 = nuevaHoja->getId();
                idH2 = nuevaHoja2->getId();

                nuevaRaiz = new VarLenIndexNode(_raiz->getId(), _raiz->getNivel() + 1, _raiz->getPadre(),
											   idH1, claveARaiz, idH2);

                delete _raiz;
                _nodoActual = _raiz = nuevaRaiz;
                return true;

            default:
                return false;
            }
		}
    }

    else { 
        _raiz = new VarLenLeafNode();
        _nNodos++;
        _nodoActual = _raiz;
        _raiz->insert(key, reference);
    }
    return true;
}

bool VarLenBPlusTree::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, VarLenNode::VARLEN_NODE_SIZE);
        // leo la cantidad de nodos
        memcpy(&_nNodos, nextByte, sizeof(int));
        return true;
    }
    return false;
}

bool VarLenBPlusTree::writeHeader()
{
    if (_filestr.is_open()) {
        // clear buffer
        memset(_buffer,'\0', VarLenNode::VARLEN_NODE_SIZE);
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_nNodos, sizeof(int));
        // volcar al archivo
        _filestr.seekg(0, ios::beg);
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, VarLenNode::VARLEN_NODE_SIZE);
        return true;
    }
    return false;
}

bool VarLenBPlusTree::readRoot()
{
    if (_filestr.is_open()) {
        _filestr.seekg(VarLenNode::VARLEN_NODE_SIZE, ios::beg);
        _filestr.read(_buffer, VarLenNode::VARLEN_NODE_SIZE);

        if (_nNodos > 1)
            _raiz = new VarLenIndexNode();
        else
            _raiz = new VarLenLeafNode();

        _raiz->read(_buffer);
        return true;
    }
    return false;
}

bool VarLenBPlusTree::writeRoot()
{
    if (_filestr.is_open()) {
        if (_raiz != 0)
            _raiz->write(_buffer);

        if (_filestr.fail())
            _filestr.clear();

        _filestr.seekg(VarLenNode::VARLEN_NODE_SIZE, ios::beg);
        _filestr.seekp(VarLenNode::VARLEN_NODE_SIZE, ios::beg);
        _filestr.write(_buffer, VarLenNode::VARLEN_NODE_SIZE);
        return true;
    }
    return false;
}

bool VarLenBPlusTree::readNode(int id, Node** node)
{
    if (_filestr.is_open()) {
        if ((*node != _raiz) && (*node != 0)) {
            writeNode(*node);
            delete (*node);
        }
        else
            *node = 0;
        
        _filestr.seekg(VarLenNode::VARLEN_NODE_SIZE * (id + 1), ios::beg);
        _filestr.read(_buffer, VarLenNode::VARLEN_NODE_SIZE);

        int nivelNodo;
        memcpy(&nivelNodo, _buffer, sizeof(int));

        if (nivelNodo != 0)
            (*node) = new VarLenIndexNode();
        else
            (*node) = new VarLenLeafNode();

        (*node)->read(_buffer);
        return true;
    }
    return false;
}

bool VarLenBPlusTree::writeNode(Node* node)
{
    if (_filestr.is_open()) {
        node->write(_buffer);

        if(_filestr.fail())
            _filestr.clear();

        _filestr.seekp(VarLenNode::VARLEN_NODE_SIZE * (node->getId() + 1), ios::beg);

        if (_filestr.fail())
            _filestr.clear();

        _filestr.write(_buffer, VarLenNode::VARLEN_NODE_SIZE);
        return true;
    }
    return false;
}

void VarLenBPlusTree::insertarEnPadre(int NroNodoPadre, int NroNodoHijo,const char* claveAlPadre)
{
    VarLenIndexNode* nuevoIndice = 0;
    VarLenIndexNode* nuevoIndice2 = 0;
	char* arreglo;
	int bytesArreglo;
	int clavesArreglo;
	char* claveASubir;
	char* claveARaiz;
    
    if (NroNodoPadre != 0) { // el padre es distinto de la raiz
        readNode(NroNodoPadre, (Node**)&_nodoActual);

        if (_nodoActual->insert(claveAlPadre, NroNodoHijo, &clavesArreglo, &arreglo, &bytesArreglo) == 2) {
            // hago el split
            nuevoIndice = static_cast<VarLenIndexNode*>((static_cast<VarLenIndexNode*>(_nodoActual))->split
									(_nNodos, arreglo, bytesArreglo, clavesArreglo, &claveASubir));

			delete(arreglo);
            _nNodos++;
            // escribo los nodos en disco
            writeNode(_nodoActual);
            writeNode(nuevoIndice);

            // actualizo el puntero al padre en los hijos del nuevo nodo indice
            actualizarPadre(nuevoIndice);

            int referenciaNuevoNodo = nuevoIndice->getId();

			int padreNuevo = nuevoIndice->getPadre();

            delete nuevoIndice; 

            insertarEnPadre(padreNuevo, referenciaNuevoNodo, claveASubir);
			
			delete(claveASubir);
            return;
        }
        return;
    }

    // si llego hasta aca es porque el que esta "rebalsado" es la raiz
    
    if (_raiz->insert(claveAlPadre, NroNodoHijo, &clavesArreglo, &arreglo, &bytesArreglo) == 2) {

        _raiz->promoteRoot( (VarLenNode**)(&nuevoIndice),
                            (VarLenNode**)(&nuevoIndice2),
                            _nNodos,_nNodos + 1,
							clavesArreglo,&arreglo,bytesArreglo,&claveARaiz);

        delete(arreglo);
        
        writeNode(nuevoIndice);
        writeNode(nuevoIndice2);

        _nNodos += 2;

        actualizarPadre(nuevoIndice);
        actualizarPadre(nuevoIndice2);

		VarLenIndexNode* nuevaRaiz = new VarLenIndexNode(_raiz->getId(),_raiz->getNivel() + 1,_raiz->getPadre(),
														nuevoIndice->getId(),claveARaiz,nuevoIndice2->getId());
        
        if (nuevoIndice) delete nuevoIndice;
        if (nuevoIndice2) delete nuevoIndice2;

		_raiz = nuevaRaiz;

		delete(claveARaiz);
														
    }
}

bool VarLenBPlusTree::actualizarPadre(VarLenIndexNode* padre)
{
    // le seteo el padre al hijo izquierdo
    readNode(padre->getHijoIzquierdo(), (Node**)&_nodoActual);
    _nodoActual->setPadre(padre->getId());

    for(int i = 1; i <= padre->getCantClaves(); ++i) {
        readNode(padre->getRef(i), (Node**)&_nodoActual);
        _nodoActual->setPadre(padre->getId());
    }

    return writeNode(_nodoActual);
}

int VarLenBPlusTree::searchPlace(const char* key)
{
    if ((_nodoActual != _raiz) && (_nodoActual != 0)) {
        writeNode(_nodoActual);
        delete _nodoActual;
        _nodoActual = _raiz;
    }

    return searchPlaceRec(key);
}

int VarLenBPlusTree::searchPlaceRec(const char* key)
{
    if (_nodoActual->getType() == VarLenNode::LEAF)
        return _nodoActual->getId();
    else {
        int indice = _nodoActual->search(key);

        if (readNode(indice, (Node**)&_nodoActual))
            return searchPlaceRec(key);

        return -1;
    }
}

int VarLenBPlusTree::search(const char* key)
{
    if (isEmpty())
        return -1;

    if ((_nodoActual != _raiz) && (_nodoActual != 0)) {
        writeNode(_nodoActual);
        delete _nodoActual;
        _nodoActual = _raiz;
    }

    while (_nodoActual->getType() != VarLenNode::LEAF) {
        int proximoALeer = _nodoActual->search(key);
        readNode(proximoALeer, (Node**)&_nodoActual);
    }

    return _nodoActual->search(key);
}

