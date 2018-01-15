/*
 * mips_assembler.cpp - convert MIPS Assembly code(Machine code) to binary code.
 *
 * TEAM 46 : 20150326 Park Si Hwan, 20150824 Hwang Jae Young
 *
 */

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <stdlib.h>
#include <sstream>
#include <fstream>

using namespace std;

/* string arrays for matching instruction to opcode and funct
 * each 1-to-1 map has same array index
 */
static string inst_r[] = {"addu","and","jr","nor","or","sltu","sll","srl","subu"};
static string inst_i[] = {"addiu","andi","beq","bne","lui","lw","la","ori","sltiu","sw"};
static string inst_j[] = {"j","jal"};
static string opcode_i[] = {"001001","001100","000100","000101","001111","100011","111111","001101","001011","101011"};
static string opcode_j[] = {"000010","000011"};
static string funct[] = {"100001","100100","001000","100111","100101","101011","000000","000010","100011"};

/* helper routines for convert assembly to binary */
static string write_text(vector<string> _text);
static string write_data(vector<string> _data);
static string match_inst2op(string s);
static string lui_handler(string r, string imm);
static string assign_regs_r(string inst, string r1, string r2, string r3);
static string assign_regs_i(string inst, string r1, string r2, string imm);
static string assign_regs_j(string inst, string addr);
static string jr_handler(string inst, string r);
int contains_r(string s);
int contains_i(string s);
int contains_j(string s);

/* helper routines for Parsing and matching address to text/data labels */
int check_addr(vector<string> v, string addr);
int check_in(vector<string> v, string addr);
vector<string> lineParser(vector<string> v, string lin);
int relative_addrFinder(int direction, int current, vector<string> v, string addr);
int absolute_addrFinder(vector <string> v, vector <string> t, string s);
string push_t2a(vector<string> t, vector<long> a, string s); 

int main(int argc, char** argv){
    
    vector<string> total; //data section + text section
    vector<string> data;
    vector<string> text;
    
    ifstream in(argv[1]);

    int check_dp=0; // position of ".data"
    int check_tp=0; // position of ".text" 
    
    /*open file*/
    string s;
    if(in.is_open()){
        while (!in.eof()){
            getline(in, s);
            total = lineParser(total, s);
        }
    }
    else {
        cout << "Cannot find such file." << endl;
	exit(1);
    }

    /* data session and text session devide */
    for(vector<string>::size_type i = 0; i < total.size(); i++){
        if(total[i] == ".data") check_dp = i;
        else if (total[i] == ".text") check_tp = i;
    }

    for(vector<string>::size_type i = 0; i < total.size(); i++){
        if(i>check_dp & i<check_tp) data.push_back(total[i]);
        if(i>check_tp)  text.push_back(total[i]);
    }

    /* make data address table to help matching data addr to data label */
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


    vector<string> tar; // save lab1:, lab2:, ..
    vector<string> prefound; // find direction

    for(vector<string>::size_type i = 0; i < text.size(); i++){
        //first, we notice that there is tar such as lab1. not tar_addr. we will find tar_addr after treat "la" instruction.
	if(text[i].at(text[i].size()-1) == ':') tar.push_back(text[i]);     
	
	if(check_addr(tar, text[i]))  prefound.push_back(text[i]); // find searching direction
	
	/* put data addr to text section ex. data1->0x10000000 */
        if(check_addr(data_tar, text[i])){
            string s = text[i];
            text.erase(text.begin() + i);
            text.insert(text.begin() + i, push_t2a(data_tar, data_tar_addr, s));
        }
	
	/* memory addressing mode to imm and register */
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
    
    /* change la - > lui, ori */
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

    /*convert target -> target address
     *we must consider that it is relative address or absolute address
     */
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
    /*look text first again 
     *erase target labels(lab1:, lab2:,..)
     */
    for(vector<string>::size_type i=0; i<text.size(); i++){
        if(check_in(tar, text[i])){
            text.erase(text.begin() + i);
        }
    }

    /* write text binary and data binary and size of text and data section */
    string text_sec = write_text(text);
    string data_sec = write_data(data);
    int text_size = text_sec.size()/8;
    int data_size = data_sec.size()/8;
	
    string text_size_str = bitset<32>(text_size).to_string();
    string data_size_str = bitset<32>(data_size).to_string();
    
    cout<<text_size_str+data_size_str+text_sec+data_sec<<endl;

    return 0;
}

/*
 * write_text - read text section(vector of string) and write corresponding binary code.
 */
static string write_text(vector<string> _text){
	string output="";

	int i=0;
	while(i<(int)_text.size()){
		if(contains_r(_text[i])){
			string instcode = match_inst2op(_text[i]);
			
			if(_text[i]=="jr"){
				output += jr_handler(instcode,_text[i+1]);
				i=i+2;
			}
			else{
				output += assign_regs_r(instcode,_text[i+1],_text[i+2],_text[i+3]);
				i=i+4;
			}
		}
		else if(contains_i(_text[i])){
			if(_text[i]=="lui"){
				output += lui_handler(_text[i+1],_text[i+2]);
				i=i+3;
			}
			else{
				string instcode = match_inst2op(_text[i]);
				output += assign_regs_i(instcode,_text[i+1],_text[i+2],_text[i+3]);
				i=i+4;
			}
		}
		else if(contains_j(_text[i])){
			string instcode = match_inst2op(_text[i]);
			output += assign_regs_j(instcode,_text[i+1]);
			i=i+2;
		}
		else i++;
	}
	
	return output;
}

/*
 * write_data - read data section(vector of string) and write corresponding binary code.
 */
static string write_data(vector<string> _data){
	string output="";

	int i=0;
	while(i<(int)_data.size()){
		
		if(_data[i].at(_data[i].size()-1) != ':' && _data[i] != ".word"){
			unsigned long data_val;
			if(_data[i].at(0)=='0'){
				if(_data[i].size()>1 && _data[i].at(1)=='x')
					data_val = strtoul(_data[i].c_str(),0,16);
				else
					data_val = atoi(_data[i].c_str());
			}
			else
				data_val = atoi(_data[i].c_str());
		
			output += bitset<32>(data_val).to_string();
		}
		i++;
	}
	
	return output;
}

/*
 * match_inst2op - read instruction and return corresponding opcode+funct(funct is only for R type).
 */
static string match_inst2op(string s){
	
	string opcode = "";
	string funct_code = "";
	
	for(int i=0;i<9;i++){
		if(s==inst_r[i]){
			opcode = "000000";
			funct_code = funct[i];  
		}
	}
	for(int i=0;i<10;i++){
		if(s==inst_i[i]) opcode = opcode_i[i];
	}
	for(int i=0;i<2;i++){
		if(s==inst_j[i]) opcode = opcode_j[i];
	}

	return opcode+funct_code;
}

/*
 * lui_handler - since lui is only one instruction in I type which has only 2 operand.
 * 		 so, we should handle lui for exception. 
 */
static string lui_handler(string r, string imm){

	r = r.erase(0,1);
	int reg = atoi(r.c_str());
	unsigned long imm_val;
	
	/* Different converting Process for hexadecimal and decimal integer strings */
	if(imm.at(0)=='0'){
		if(imm.size()>1 && imm.at(1)=='x')
			imm_val = strtoul(imm.c_str(),0,16); // unsigned long
		else
			imm_val = atoi(imm.c_str());
	}
	else
		imm_val = atoi(imm.c_str()); // int -> unsigned long (maybe autocasting)

	
	string rt = bitset<5>(reg).to_string();
	string imm_binary = bitset<16>(imm_val).to_string();
	
	return "00111100000"+rt+imm_binary; // lui always has 00000 in rs.
	
}

/*
 * assign_reg_r - read registers and assign them into binary code.(for R format)
 */
static string assign_regs_r(string inst, string r1, string r2, string r3){
	
	/* string to integer */
	r1 = r1.erase(0,1); // delete $
	r2 = r2.erase(0,1);
	
	int reg1 = atoi(r1.c_str()); // convert string to integer
	int reg2 = atoi(r2.c_str());
	int shamt = 0; // initailize shamt and reg3 to 0 then when shamt or regs is not modified, it remains 0
	int reg3 = 0;
	
	if(r3.at(0) != '$'){
	// if third register is not a register, this instruction is sll or srl.
		shamt = atoi(r3.c_str());
		string rs = "00000";
		string rt = bitset<5>(reg2).to_string();
		string rd = bitset<5>(reg1).to_string();
		string shamt_binary = bitset<5>(shamt).to_string();

		return inst.insert(6,rs+rt+rd+shamt_binary);
	}
	else{
		r3 = r3.erase(0,1);
		reg3 = atoi(r3.c_str());
	}
	
	/* integer to binary string */
	string rd = bitset<5>(reg1).to_string(); // insert to rd
	string rs = bitset<5>(reg2).to_string(); // insert to rs
	string rt = bitset<5>(reg3).to_string(); // insert to rt
	string shamt_binary = bitset<5>(shamt).to_string(); //insert to shamt
	
	return inst.insert(6,rs+rt+rd+shamt_binary); // insert rs+rt+rd+shamt between opcode+funct
}

/*
 * assign_regs_i - read registers and immediate value and assign them into binary code.(for I format)
 */
static string assign_regs_i(string inst, string r1, string r2, string imm){
	
	/* string to integer */
	r1 = r1.erase(0,1);
	r2 = r2.erase(0,1);
	
	int reg1 = atoi(r1.c_str());
	int reg2 = atoi(r2.c_str());
	unsigned long imm_val;
	string rs,rt;
	
	/* check whether imm is hex or not */
	if(imm.at(0)=='0'){
		if(imm.size()>1 && imm.at(1)=='x')
			imm_val = strtoul(imm.c_str(),0,16); // unsigned long
		else
			imm_val = atoi(imm.c_str());
	}
	else
		imm_val = atoi(imm.c_str()); // int -> unsigned long (maybe autocasting)
	
	/* integer to binary string */
	if(inst=="000100" || inst=="000101"){
		/* handle bne or beq : branch instructions have different register ordering */
		rs = bitset<5>(reg1).to_string();
		rt = bitset<5>(reg2).to_string();
	}
	else{
		rs = bitset<5>(reg2).to_string();
		rt = bitset<5>(reg1).to_string();
	}

	string imm_binary = bitset<16>(imm_val).to_string();

	return inst.insert(6,rs+rt+imm_binary);
}

/*
 * assign_regs_j - read address and assign it into binary code.(for J type)
 */
static string assign_regs_j(string inst, string addr){
	
	//addr is hexadecimal
	unsigned long addr_val = atoi(addr.c_str());
	string addr_binary = bitset<26>(addr_val).to_string();
	
	return inst.insert(6,addr_binary);
}

/*
 * jr_handler - since jr is the only one instruction in R type which has only 1 operand.
 * 		so, we should handle it for exception.
 */
static string jr_handler(string inst, string r){
	r = r.erase(0,1);
	int reg = atoi(r.c_str());
	string rs = bitset<5>(reg).to_string();
	return inst.insert(6,rs+"000000000000000");
}

/*
 * contains - return 1 if there is s in arr. if not, return 0
 */
int contains_r(string s){	
	for(int i=0; i<9 ;i++){
		if(inst_r[i]==s) return 1;
	}
	return 0;
}

int contains_i(string s){
	for(int i=0;i<10;i++){
		if(inst_i[i]==s) return 1;
	}
	return 0;
}

int contains_j(string s){
	for(int i=0;i<2;i++){
		if(inst_j[i]==s) return 1;
	}
	return 0;
}

/*
 * absoulte_addrFinder - find absolute address of given string s (addr label in text section).
 * 			 find it and divide by 4 because it will used for j and jal instructions.
 */
int absolute_addrFinder(vector <string> v, vector <string> t, string s){
    int addr = 0x400000;
    
    string ins_arr[] = {"addiu", "addu", "and", "andi", "beq", "bne", "j", "jal", "jr", "lui", "lw", "la", "nor", "or", "ori", "sltiu", "sltu", "sll", "srl", "sw", "subu"};    
    int ins_arrSize = sizeof(ins_arr)/sizeof(string);
    
    vector<string> ins;
    ins.insert(ins.begin(), ins_arr, ins_arr + ins_arrSize);

    for(vector<string>::size_type i = 0; i < v.size(); i++){
        if(check_in(ins, v[i]))	addr+=4;
        else if(check_addr(t,s)){
            string restore = v[i];
            
	    if(restore.erase(v[i].size() - 1, 1) == s)	return addr/4;
            else continue;
        }
        else continue;
    }
}

/*
 * push_t2a - helper function for translate addr label to addr.
 */
string push_t2a(vector<string> t, vector<long> a, string s){
    
    for(vector<string>::size_type i = 0; i < t.size(); i++){
        string check = t[i];
        if(check.erase(check.size()-1, 1) == s){
                stringstream ss;
                ss<<a[i];
                return  ss.str();
        }
    }
}

/*
 * relative_addrFinder - find Next-PC relative address for bne and beq instructions.
 */
int relative_addrFinder(int direction, int current, vector<string> v, string addr){

    int count = 0;
    
    string ins_arr[] = {"addiu", "addu", "and", "andi", "beq", "bne", "j", "jal", "jr", "lui", "lw", "la", "nor", "or", "ori", "sltiu", "sltu", "sll", "srl", "sw", "subu"};
    
    int ins_arrSize = sizeof(ins_arr)/sizeof(string);
    
    vector<string> ins;
    ins.insert(ins.begin(), ins_arr, ins_arr + ins_arrSize);

    while(1){
        if(direction == 1){
	    /* searching to downward */
            string restore = v[current];
            if (addr == v[current].erase(v[current].size()-1, 1))   return count;
            else if(check_in(ins, restore))  count++;
            v[current] = restore;
            current++;
        }
        if(direction == -1){
	    /* searching to upward */
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

/*
 * check_in - check addr is in v
 */
int check_in(vector<string> v, string addr){
    for(vector<string>::size_type i=0; i<v.size(); i++){
         if(addr == v[i])    return 1;
    }
    return 0;
}

/*
 * check_addr - check addr is in v (However, only compare label - ignore ':' ex. lab1:, lab2:).
 */
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

/*
 * lineParser - Parsing the line.
 */
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
            if (i == length-1){
                v.push_back(word);
                word.erase(0, word.length());
            }
        }
            
    }
    return v;
}
