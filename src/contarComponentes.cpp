
#include <string>
#include <iostream>

using std::cout;
using std::cin;
using std::string;

int contarComponentes(const string& a_Target)
{
  int cantComponentes = 0;
  unsigned int length = a_Target.length();
  unsigned int index = 0;

  while(index < length)
    {
      index = a_Target.find_first_of("/",index);      
      cantComponentes++;
      if(index < length)
			index++;
    }

  return cantComponentes;
  
}

string obtenerComponente(const string a_Target,int componenteAObtener)
{
	int cantidadComponentes = contarComponentes(a_Target);
	
	string res;	

	if(componenteAObtener > cantidadComponentes)
 		res = "";
	
	else
	{
		int componenteLeida = 0;
		unsigned int comienzo = 0;
		unsigned int fin = 0;
	

		while(componenteLeida != componenteAObtener)
		{
			comienzo = fin;
			fin = a_Target.find_first_of("/",fin);
			componenteLeida++;
			
			if(fin < a_Target.length())
				fin++;
		}

		res = a_Target.substr(comienzo,fin - comienzo - 1);		
	}

	return res;	
}

int  main(int argv, char** argc)
{
	string path;
	int componenteAObtener;
	cout<<"ingrese el path: "<<"\n";
	cin>>path;
	cout<<"\n";
	cout<<"que componente desea obtener?"<<"\n";
	cin>>componenteAObtener;
	cout<<"\n";

	cout<<"componente obtenida: "<<obtenerComponente(path,componenteAObtener)<<"\n";

	return 0;
}
