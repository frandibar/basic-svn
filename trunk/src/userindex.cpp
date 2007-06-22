// userindex.cpp

#include "userindex.h"

using std::ios;

// constructor
UserIndex::UserIndex() : _raiz(0), _nodoActual(0), _nNodos(0), _isOpen(false)
{
    _buffer = new char[UserNode::NODE_SIZE];
}

UserIndex::~UserIndex()
{
    delete _buffer;
    delete _nodoActual;
    delete _raiz;
}

bool UserIndex::readHeader()
{
    if (_filestr.is_open()) {
        char* nextByte = _buffer;
        _filestr.seekg(0, ios::beg);
        _filestr.read(_buffer, UserNode::NODE_SIZE);
        // leo la cantidad de nodos
        memcpy(&_nNodos, nextByte, sizeof(int));
        return true;
    }
    return false;
}

bool UserIndex::readRoot()
{
    if (_filestr.is_open()) {
        _filestr.seekg(UserNode::NODE_SIZE, ios::beg);
        _filestr.read(_buffer, UserNode::NODE_SIZE);

        if (_nNodos > 1)
            _raiz = new IndexUserNode();
        else
            _raiz = new LeafUserNode();

        _raiz->read(_buffer);
        return true;
    }
    return false;
}

bool UserIndex::writeHeader()
{
    if (_filestr.is_open()) {
        // clear buffer
        memset(_buffer,'\0', UserNode::NODE_SIZE);
        char* nextByte = _buffer;
        // volcar en _buffer la cantidad de nodos
        memcpy(nextByte, &_nNodos, sizeof(int));
        // volcar al archivo
        _filestr.seekg(0, ios::beg);
        _filestr.seekp(0, ios::beg);
        _filestr.write(_buffer, UserNode::NODE_SIZE);
        return true;
    }
    return false;
}

bool UserIndex::writeRoot()
{
    if (_filestr.is_open()) {
        if (_raiz != 0)
            _raiz->write(_buffer);

        if (_filestr.fail())
            _filestr.clear();

        _filestr.seekg(UserNode::NODE_SIZE, ios::beg);
        _filestr.seekp(UserNode::NODE_SIZE, ios::beg);
        _filestr.write(_buffer, UserNode::NODE_SIZE);
        return true;
    }
    return false;
}

bool UserIndex::readNode(int idNode, UserNode** node)
{
    if (_filestr.is_open()) {
        if ((*node != _raiz) && (*node != 0)) {
            writeNode(*node);
            delete (*node);
        }
        else
            *node = 0;
        
        _filestr.seekg(UserNode::NODE_SIZE * (idNode + 1), ios::beg);
        _filestr.read(_buffer, UserNode::NODE_SIZE);

        int nivelNodo;
        memcpy(&nivelNodo, _buffer, sizeof(int));

        if (nivelNodo != 0)
            (*node) = new IndexUserNode();
        else
            (*node) = new LeafUserNode();

        (*node)->read(_buffer);
        return true;
    }
    return false;
}

bool UserIndex::writeNode(UserNode* nodo)
{
    if (_filestr.is_open()) {
        nodo->write(_buffer);

        if(_filestr.fail())
            _filestr.clear();

        _filestr.seekp(UserNode::NODE_SIZE * (nodo->getId() + 1), ios::beg);

        if (_filestr.fail())
            _filestr.clear();

        _filestr.write(_buffer, UserNode::NODE_SIZE);
        return true;
    }
    return false;
}

bool UserIndex::create(const string& a_Filename)
{
    if (_isOpen)
        return false;

    _filestr.open(a_Filename.c_str(), ios::out | ios::in | ios::binary);

	if (!_filestr.is_open()) {
		_filestr.open(a_Filename.c_str(), ios::out | ios::binary);
		_filestr.close();

		_filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
		if (!_filestr.is_open()) {
            _isOpen = false;
			return false;
        }
	}

    _nNodos = 0;
    _isOpen = writeHeader();

    return _isOpen;
}

bool UserIndex::destroy()
{
    if (_isOpen)
        return false;

    int ret = remove(_filename.c_str());
    return (ret == 0);
}

bool UserIndex::open(const string& a_Filename)
{
    if (_isOpen) 
        return false;

    _filestr.open(a_Filename.c_str(), ios::in | ios::out | ios::binary);
    _isOpen = _filestr.is_open() && readHeader();

    if (_nNodos > 0)
        _isOpen = _isOpen && readRoot();

    if (_isOpen) {
        _nodoActual = _raiz;
        _filename = a_Filename;
    }
    return _isOpen;
}

bool UserIndex::close()
{
    if (!_isOpen)
        return true;

    bool ret = true;
    if (writeHeader()) {
        if((_raiz != 0)  && writeRoot())
         if (_raiz != _nodoActual)
            ret = writeNode(_nodoActual);
    }
    else 
        ret = false;

    _filestr.close();
    _isOpen = _filestr.is_open();
    return (!_isOpen && ret);
}

int UserIndex::searchPlace(const char* key)
{
    if ((_nodoActual != _raiz) && (_nodoActual != 0)) {
        writeNode(_nodoActual);
        delete _nodoActual;
        _nodoActual = _raiz;
    }

    return searchPlaceRec(key);
}

int UserIndex::searchPlaceRec(const char* key)
{
    if (_nodoActual->getType() == UserNode::LEAF)
        return _nodoActual->getId();
    else {
        int indice = _nodoActual->search(key);

        if (readNode(indice, &_nodoActual))
            return searchPlaceRec(key);

        return -1;
    }
}

bool UserIndex::actualizarPadre(IndexUserNode* padre)
{
    // le seteo el padre al hijo izquierdo
    readNode(padre->getHijoIzquierdo(), &_nodoActual);
    _nodoActual->setPadre(padre->getId());

    for(int i = 1; i <= padre->getCantClaves(); ++i) {
        readNode(padre->getRef(i), &_nodoActual);
        _nodoActual->setPadre(padre->getId());
    }

    return writeNode(_nodoActual);
}

void UserIndex::insertarEnPadre(int idNodoPadre, int idNodoHijo, char* claveAlPadre)
{
    IndexUserNode* nuevoIndice = 0;
    IndexUserNode* nuevoIndice2 = 0;
	char* arreglo;
	int bytesArreglo;
	int clavesArreglo;
	char* claveASubir;
	char* claveARaiz;
    
    if (idNodoPadre != 0) { // el padre es distinto de la raiz
        readNode(idNodoPadre, &_nodoActual);

        if (_nodoActual->insert(claveAlPadre, idNodoHijo, &clavesArreglo, &arreglo, &bytesArreglo) == 2) {
            // hago el split
            nuevoIndice = static_cast<IndexUserNode*>((static_cast<LeafUserNode*>(_nodoActual))->split
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
    
    if (_raiz->insert(claveAlPadre, idNodoHijo, &clavesArreglo, &arreglo, &bytesArreglo) == 2) {

        _raiz->promoteRoot( (UserNode**)(&nuevoIndice),
                            (UserNode**)(&nuevoIndice2),
                            _nNodos,_nNodos + 1,
							clavesArreglo,&arreglo,bytesArreglo,&claveARaiz);

        delete(arreglo);
        
        writeNode(nuevoIndice);
        writeNode(nuevoIndice2);

        _nNodos += 2;

        actualizarPadre(nuevoIndice);
        actualizarPadre(nuevoIndice2);

		IndexUserNode* nuevaRaiz = new IndexUserNode(_raiz->getId(),_raiz->getNivel() + 1,_raiz->getPadre(),
														nuevoIndice->getId(),claveARaiz,nuevoIndice2->getId());
        
        if (nuevoIndice) delete nuevoIndice;
        if (nuevoIndice2) delete nuevoIndice2;

		_raiz = nuevaRaiz;

		delete(claveARaiz);
														
    }
}

bool UserIndex::insert(const char* key, int reference)
// returns true if insertion was successfull
{
    LeafUserNode* hojaNueva;
    LeafUserNode* hnoDerecho;
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
                hojaNueva = static_cast<LeafUserNode*>((static_cast<LeafUserNode*> (_nodoActual))->split
									(_nNodos,arreglo,bytesArreglo,clavesArreglo,&claveAlPadre));

				// libero el arreglo
				delete(arreglo);

                // tengo que levantar el hno derecho de la nueva hoja y setearle la nueva hoja 
                // como su hno izquierdo
                if (hojaNueva->getHnoDerecho() > 0) {
                    hnoDerecho = 0;
                    readNode(hojaNueva->getHnoDerecho(), (UserNode**)&hnoDerecho);
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
            IndexUserNode* nuevaRaiz;
            int idH1;
            int idH2;
            LeafUserNode* nuevaHoja;
            LeafUserNode* nuevaHoja2;
            switch (_raiz->insert(key,reference, &clavesArreglo, &arreglo, &bytesArreglo)) {
            case 1:
                return true;

            case 2:
                nuevaHoja = 0;
                nuevaHoja2 = 0;
                
                // creo 2 nodos hoja nuevos para hacer el split y hacer que la nueva raiz sea indice
                _raiz->promoteRoot( (UserNode**)(&nuevaHoja),
                                    (UserNode**)(&nuevaHoja2),
                                    _nNodos, _nNodos + 1,
									clavesArreglo, &arreglo, bytesArreglo, &claveARaiz);

                _nNodos += 2;

                // escribo las 2 nuevas hojas
                writeNode(nuevaHoja);
                writeNode(nuevaHoja2);

                idH1 = nuevaHoja->getId();
                idH2 = nuevaHoja2->getId();

                nuevaRaiz = new IndexUserNode(_raiz->getId(), _raiz->getNivel() + 1, _raiz->getPadre(),
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
        _raiz = new LeafUserNode();
        _nNodos++;
        _nodoActual = _raiz;
        _raiz->insert(key, reference);
    }
    return true;
}

int UserIndex::search(const char* key)
{
    if (isEmpty())
        return -1;

    if ((_nodoActual != _raiz) && (_nodoActual != 0)) {
        writeNode(_nodoActual);
        delete _nodoActual;
        _nodoActual = _raiz;
    }

    while (_nodoActual->getType() != UserNode::LEAF) {
        int proximoALeer = _nodoActual->search(key);
        readNode(proximoALeer, &_nodoActual);
    }

    return _nodoActual->search(key);
}

