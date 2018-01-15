#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <sstream>

using namespace std;


int check_addr(vector<string> v, string addr);
int check_in(vector<string> v, string addr);
vector<string>  lineParser(vector<string> v, string lin);
int relative_addrFinder(int direction, int current, vector<string> v, string addr);
int absolute_addrFinder(vector <string> v, vector <string> t, string s);
string push_t2a(vector<string> t, vector<long> a, string s); 

int main(int argc, char** argv)
{
    vector<string> total;
    vector<string> data;
    vector<string> text;
    ifstream in(argv[1]);
    int check_dp=0;
    int check_tp=0;
    string s;
    if(in.is_open()){
        while (!in.eof()){
            
           
            getline(in, s);
            total = lineParser(total, s);
            
        }
    }
    else {
        cout << "Cannot find such file." << endl;
    }
    //data session and text session devide

    for(vector<string>::size_type i = 0; i < total.size(); i++){
        if(total[i] == ".data") check_dp = i;
        else if (total[i] == ".text") check_tp = i;
    }

    for(vector<string>::size_type i = 0; i < total.size(); i++){
        if(i>check_dp & i<check_tp) data.push_back(total[i]);
        if(i>check_tp)  text.push_back(total[i]);
    }
    //.data's value to test's value of .data
    vector<string> data_tar;
    vector<long> data_tar_addr;
    long data_addr_checker = 0;
    long pre_data_addr = 0x10000000;
    long length = (long) data.size();
    for(long i=0; i < length; i++){
        if(data[i].at(data[i].size()-1) == ':'){
            if(i == 0){
                data_tar.push_back(data[i]);
                data_tar_addr.push_back(0x10000000);
          
            }
            else{
                long godputer = pre_data_addr + (i-data_addr_checker-1) *2;
                data_tar.push_back(data[i]);
                data_tar_addr.push_back(godputer);
                
                pre_data_addr = pre_data_addr + ((i - data_addr_checker - 1) * 2);
                data_addr_checker = i;
            }
        }
    }


    vector<string> tar;
    vector<string> prefound;

    
    
    for(vector<string>::size_type i = 0; i < text.size(); i++){
        if(text[i].at(text[i].size()-1) == ':') tar.push_back(text[i]);     //first, we notice that there is tar such as lab1. not tar_addr. we will find tar_addr after treat "la" instruction.
        
        if(check_addr(tar, text[i]))  prefound.push_back(text[i]);

        if(check_addr(data_tar, text[i])){
            string s = text[i];
            text.erase(text.begin() + i);
            text.insert(text.begin() + i, push_t2a(data_tar, data_tar_addr, s));
        }

        if(text[i] == "lw" | text[i] == "sw"){
            string target = text[i+2];
            text.erase(text.begin()+i+2);       //erase
            string sub1 = target.substr(0, target.find('('));
            string sub2 = target.substr(target.find('(')+1, target.find(')')-target.find('(')-1);
            string add[2] = {sub2, sub1};
            vector<string>::iterator it = text.begin();
            text.insert(it+i+2, add, add+2);    //append
        }
    }
    
    //change la - > lui, ori 
    for(vector<string>::size_type i = 0; i < text.size(); i++)
    {
        if(text[i] == "la"){
            
            int lui_checker = atoi(text[i+2].c_str());
            string reg = text[i+1];

            stringstream ss1;
            stringstream ss2;
           
            ss1<<(lui_checker>>16);
            ss2<<(lui_checker&0xffff);
            string lui_part = ss1.str();
            string ori_part = ss2.str();

            if(lui_checker & 0xffff){
                string add[7] = {"lui", reg, lui_part, "ori", reg, reg, ori_part};
                text.erase(text.begin() + i, text.begin() + i + 3);
                text.insert(text.begin()+i, add, add + 7);
            }
            else{
                string add[3] = {"lui", reg, lui_part};
                text.erase(text.begin() + i, text.begin() + i + 3);
                text.insert(text.begin() + i, add, add + 3);
            }
        }
    }

//convert target -> target address
//we must consider that it is relative address or absolute address
    for(vector<string>::size_type i=0; i<text.size(); i++){
        if(check_addr(tar, text[i])){
            if(text[i-1]=="j" || text[i-1]=="jal"){
                int abs_addr_int = absolute_addrFinder(text, tar, text[i]);
                stringstream ss;
                ss<<abs_addr_int;
                string abs_addr = ss.str();

                text.erase(text.begin() + i);
                text.insert(text.begin() + i, abs_addr);
            }
            else{
                int rel_addr_int = 0;
                if(check_in(prefound, text[i]))
                    rel_addr_int = relative_addrFinder(-1, i, text, text[i]);
                else
                    rel_addr_int = relative_addrFinder(1, i, text, text[i]);

                    stringstream ss;
                    ss<<rel_addr_int;
                    string rel_addr = ss.str();

                    text.erase(text.begin()+i);
                    text.insert(text.begin()+i, rel_addr);
                
                }
        }
    }
//look text first again 
//erase target address
    for(vector<string>::size_type i=0; i<text.size(); i++){
        if(check_in(tar, text[i])){
            text.erase(text.begin() + i);
        }
    }



    //check session
    cout << "total session element" << endl;
    for(vector<string>::size_type i=0; i<total.size(); i++){
        cout<<total[i]<<endl;
    }

    
    cout << "data session element" << endl;
    for(vector<string>::size_type i = 0; i<data.size() ; i++){
        cout<<data[i] << endl;
    }
    cout << "text session element" << endl;
    for(vector<string>::size_type i = 0; i<text.size() ; i++){
        cout<<text[i] << endl;
    }

    cout << "data size : "<< text.size() << endl;
    return 0;
} 

int absolute_addrFinder(vector <string> v, vector <string> t, string s){
    int addr = 0x400000;
    
    string ins_arr[] = {"addiu", "addu", "and", "andi", "beq", "bne", "j", "jal", "jr", "lui", "lw", "la", "nor", "or", "ori", "sltiu", "sltu", "sll", "srl", "sw", "subu"};    
    int ins_arrSize = sizeof(ins_arr)/sizeof(string);
    vector<string> ins;
    ins.insert(ins.begin(), ins_arr, ins_arr + ins_arrSize);

    for(vector<string>::size_type i = 0; i < v.size(); i++){
        if(check_in(ins, v[i]))    addr+=4;
        else if(check_addr(t,s)){
            string restore = v[i];
            if(restore.erase(v[i].size() - 1, 1) == s)  return addr/4;
            else    continue;
        }
        else    continue;
    }
}


string push_t2a(vector<string> t, vector<long> a, string s){
    cout <<"a size"<< a.size() << endl;
    cout << "t size " <<t.size()<<endl;
    for(vector<string>::size_type i = 0; i < t.size(); i++){
        cout << "count : "<< i << endl;
        string check = t[i];
        if(check.erase(check.size()-1, 1) == s){
                stringstream ss;
                ss<<a[i];
                return  ss.str();
        }
    }
    
}

int relative_addrFinder(int direction, int current, vector<string> v, string addr){

    int count = 0;
    
    string ins_arr[] = {"addiu", "addu", "and", "andi", "beq", "bne", "j", "jal", "jr", "lui", "lw", "la", "nor", "or", "ori", "sltiu", "sltu", "sll", "srl", "sw", "subu"};
    
    int ins_arrSize = sizeof(ins_arr)/sizeof(string);
    
    vector<string> ins;
    ins.insert(ins.begin(), ins_arr, ins_arr + ins_arrSize);

    while(1){
        if(direction == 1){
            string restore = v[current];
            if (addr == v[current].erase(v[current].size()-1, 1))   return count;
            else if(check_in(ins, restore))  count++;
            v[current] = restore;
            current++;
        }
        if(direction == -1){
            string restore = v[current];
            if (addr == v[current].erase((v[current].size()-1), 1))   return count;
            else if(check_in(ins, restore)){
                count--;
            }
            v[current] = restore;
            current--;
        }
    }
    return count;
}


int check_in(vector<string> v, string addr){
    for(vector<string>::size_type i=0; i<v.size(); i++){
         if(addr == v[i])    return 1;
    }
    return 0;
}

int check_addr(vector<string> v, string addr){
   
    if(addr.at(0) == '$')   return 0;
    for(vector<string>::size_type i = 0; i<v.size(); i++){
        string restore = v[i];
        if (addr == v[i].erase(v[i].size()-1, 1)){
            v[i] = restore;
            return 1;
        }
        v[i] = restore;
    }
    return 0;
}

vector<string> lineParser(vector<string> v, string line)
{
    string word;
    
    int length = line.length();
    for(int i = 0; i < length; i++){
        if(line.at(i) == ' ' || line.at(i) == '	'){
            if ((i==0) || ((i!=0) & ((line.at(i-1) == ',')|(line.at(i-1) == ' ')))){
                word.erase(0, word.length());
                continue;
            }
            else{
               
                v.push_back(word);               
                word.erase(0, word.length());
                
            }
        }
        else if (line.at(i) == ','){
            
            v.push_back(word);
            word.erase(0, word.length());
        }
        else{
            word += line.at(i);
            if (i == length-1)   
            {
                
                v.push_back(word);
                word.erase(0, word.length());
            }
        }
            
    }
    return v;
}
