/*
 * Simple benchmark to compare different hash table implementations
 * 
 */

#include <sys/time.h>
#include <ctime>
#include <iostream>
#include <cassert>
#include <ext/hash_map>
#include <string>
#include <bitset>
using namespace std;



//http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}


//http://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c
typedef unsigned long long uint64;
uint64 GetTimeMs64(){
    struct timeval tv;

    gettimeofday(&tv, NULL);

    uint64 ret = tv.tv_usec;
    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
    ret /= 1000;

    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);

    return ret;
}

struct hashfun_str {
    size_t operator()(const std::string& s) const {
        assert(sizeof(int) == 4 && sizeof(char) == 1); 
        std::cout << "Call hashfun_str" << std::endl;
        const int* p = reinterpret_cast<const int*>(s.c_str()); 
        size_t v = 0;
        int n = s.size() / 4;
        for (int i = 0; i < n; i++, p++) {
            //      v ^= *p;
            v ^= *p << (4 * (i % 2)); // note) 0 <= char < 128
        }
        int m = s.size() % 4;
        for (int i = 0; i < m; i++) {
            v ^= s[4 * n + i] << (i * 8);
        }
        return v;
    }
};


int main(){

    srand(time(NULL));

    typedef __gnu_cxx::hash_map<std::string, int, hashfun_str> map_type;
    map_type str2id;


    uint64 start = GetTimeMs64();
    for(int i=0; i < 30000; i++){
        char random_str [15] = "";
        gen_random(random_str, 15);
        std::string random_str_cpp(random_str);
        str2id[random_str] = rand() % 2; // could be another number
    }
    uint64 end = GetTimeMs64();
    uint64 current_time = (end - start);
    std::cout << "Time : " << current_time << std::endl;


    std::cout << str2id.size();

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
