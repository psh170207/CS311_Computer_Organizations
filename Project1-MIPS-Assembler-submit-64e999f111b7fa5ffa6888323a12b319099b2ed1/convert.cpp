#include <iostream>
#include <bitset>
#include <stdlib.h>

using namespace std;

int main(){
	string str;
	cin>>str;
	cout<<str<<"\n";


	str = str.erase(0,1);
	int num = atoi(str.c_str());
	cout<<num<<"\n";

	string binary = bitset<5>(num).to_string();
	string s = "000000";
	s.insert(6,"test"+binary);
	cout<<s<<"\n";

	unsigned long decimal = bitset<5>(binary).to_ulong();
	cout<<decimal<<"\n";
	/*
	unsigned long hex_val;

	if(str.at(1)=='x')
		hex_val = strtoul(str.c_str(),0,16);
	else
		hex_val = atoi(str.c_str());
	
	cout<<hex_val<<"\n";

	string binary = bitset<16>(hex_val).to_string();
	cout<<binary<<"\n";
	*/
	return 0;	
}
