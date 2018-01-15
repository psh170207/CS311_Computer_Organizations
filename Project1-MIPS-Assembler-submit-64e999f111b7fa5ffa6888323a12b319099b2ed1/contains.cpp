#include <iostream>
#include <string>
#include <stdlib.h>


using namespace std;

int main(){

	static string arr[] = {"161","12416",""};

	for(int i=0;i<sizeof(arr)/sizeof(string);i++){
		if(arr[i] == "12416") std::cout<<sizeof(arr)/sizeof(string)<<"\n";
	}
	
	return 0;
}
