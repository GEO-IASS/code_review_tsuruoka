/*
 * Simple benchmark to compare gnu+custom hash function and C11 hash table implementations
 * 
 */

#include <sys/time.h>
#include <ctime>
#include <iostream>
#include <cassert>
#include <ext/hash_map>
#include <string>
#include <bitset>
#include <unordered_map>
#include <fstream>

#define USE_HASH_C11

#ifdef USE_HASH_C11
#define OUTPUT  std::string("hashc11.csv")
#else
#define OUTPUT  std::string("hashfun_str.csv")
#endif

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

// http://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/maxent/ -- maxent.h
struct hashfun_str {
    size_t operator()(const std::string& s) const {
        assert(sizeof(int) == 4 && sizeof(char) == 1); 
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

    #ifdef USE_HASH_C11
    typedef std::unordered_map<std::string, int, hashfun_str> map_type;
    #else
    typedef __gnu_cxx::hash_map<std::string, int, hashfun_str> map_type;
    #endif
    std::ofstream csv_file;
    csv_file.open(OUTPUT);

    map_type str2id;

    uint64 tick_time[100];
    int tick_string[100];
    int j = 0;

    uint64 start = GetTimeMs64();

    for(int i=1; i <= 5000000; i++){
    
        char random_str [15] = "";
        gen_random(random_str, 15);
        std::string random_str_cpp(random_str);
        str2id[random_str] = rand() % 2; // could be another number

        // record time
        if(!(i % 50000)){
            uint64 end = GetTimeMs64();
            uint64 current_time = (end - start);
            tick_time[j] = current_time;
            tick_string[j] = i;
            j++;
        }

    }

    csv_file << "ms" << "," << "nb_string" << std::endl;
    for(int i=0; i < 100; i++)
        csv_file << tick_time[i] << "," << tick_string[i] << std::endl;

    csv_file.close();

    return 0;
}
