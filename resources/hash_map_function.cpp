/*
 *
 * Copyright (c) 2016, Fauconnier
 * All rights reserved.
 * 
 * Under FreeBSD License
 * 
 */

#include <iostream>
#include <cassert>
#include <ext/hash_map>
#include <string>
#include <bitset>
using namespace std;

struct hashfun_str
{
    size_t operator()(const std::string& s) const {
        assert(sizeof(int) == 4 && sizeof(char) == 1); // See if system is ok
        const int* p = reinterpret_cast<const int*>(s.c_str()); 
        size_t v = 0;

        int n = s.size() / 4; 
        for (int i = 0; i < n; i++, p++) {
            std::cout << "n(" << i << ")" << std::endl;
            v ^= *p << (4 * (i % 2)); // note) 0 <= char < 128
        }

        int m = s.size() % 4; 
        for (int i = 0; i < m; i++) {
            std::cout << "m(" <<  i << ")" << std::endl;
            v ^= s[4 * n + i] << (i * 8);
        }
        std::cout << "string: " << s << std::endl;
        std::cout << "binary of v : " <<std::bitset<64>(v) << std::endl;
        //std::cout << "size_t returned of " << s << " is " << v << std::endl;
        //std::cout << std::endl;
        return v;
    }
};



int main(){

    typedef __gnu_cxx::hash_map<std::string, int, hashfun_str> map_type;
    map_type str2id;

    str2id["A"] = 15;

    std::string s = "bbb";
    size_t v = 0;
    int n = s.size() / 4; 
    int m = s.size() % 4; 
    std::cout << std::bitset<64>(s[8])  << std::endl;

    for (int i = 0; i < m; i++) {
        int value = 4 * n +i;
        std::cout << "m(" <<  i << ")" << "=" << value << std::endl;
        v ^= s[4 * n + i] << (i * 8);
    }

    std::cout << std::bitset<64>(v)  << std::endl;

    //map_type::iterator it;
    //std::cout << std::endl;
    //for(it = str2id.begin(); it!= str2id.end(); it++){
        //std::cout << "#iterator:#" << it->first << "-" << it->second << std::endl;
    //}



    //map_type::const_iterator j = str2id.find(mystring); // find returns an iterator 
    //// pas trouvé
    //if(j == str2id.end()){
        //std::cout << "Pas trouvé " << mystring << std::endl;    
    //}


    return 0;
}
