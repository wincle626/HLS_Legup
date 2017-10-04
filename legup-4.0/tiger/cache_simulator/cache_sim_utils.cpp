#include "cache_sim_utils.h"
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <math.h>
using namespace std;

int get_line_size (int argc, char **argv)
{
    string line_size;
    int i;
    bool line_size_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-linesize")==0) && (i< (argc+1)) ){
            line_size = argv[i+1];
            line_size_set = true;
            cout<<"Line size: "<<line_size<<" bytes."<<endl;
        }
    }
    if( !line_size_set ){
        cout << "Line size not set. Using default line size 16 bytes"<<endl;
        return 16;
    }
    return atoi(line_size.c_str());
}

int get_ways_per_set (int argc, char **argv)
{
    string ways;
    int i;
    bool line_size_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-ways")==0) && (i< (argc+1)) ){
            ways = argv[i+1];
            line_size_set = true;
            cout<<"Ways/set (if == 1, it means direct mapped): "<<ways<<"."<<endl;
        }
    }
    if( !line_size_set ){
        cout << "Number of ways not set. Using default of 1 (direct mapped)"<<endl;
        return 1;
    }
    return atoi(ways.c_str());
}

int get_prefetch (int argc, char **argv)
{
    string prefetchnum;
    int i;
    bool line_size_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-prefetch")==0) && (i< (argc+1)) ){
            prefetchnum = argv[i+1];
            line_size_set = true;
            cout<<"Prefetch: "<<prefetchnum<<" lines of cache ahead."<<endl;
        }
    }
    if( !line_size_set ){
        cout << "Prefetch not set. Using default of 0 (no prefetching)"<<endl;
        return 0;
    }
    return atoi(prefetchnum.c_str());
}

int get_cache_size (int argc, char **argv)
{
    string cache_size;
    int i;
    bool cache_size_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-cachesize")==0) && (i< (argc+1)) ){
            cache_size = argv[i+1];
            cache_size_set = true;
            cout<<"Cache size: "<<cache_size<<"KB."<<endl;
        }
    }
    if( !cache_size_set ){
        cout << "Cache size not set. Using default cache size 8 KB"<<endl;
        return 8;
    }
    return atoi(cache_size.c_str());
}

int get_file_name(int argc, char **argv, string* file_name)
{
    int i;
    bool file_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-file")==0) && (i< (argc+1)) ){
            *file_name = argv[i+1];
            file_set = true;
            cout<< "Using file: "<<*file_name<<endl;
        }
    }
    if( !file_set ){
        cout << "Error: File not specified."<<endl;
        return -1;
    }
    return 0;
}

bool get_savefile(int argc, char **argv, string* file_name)
{
    int i;
    bool savefile_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-savecsv")==0) && (i< (argc+1)) ){
            *file_name = argv[i+1];
            savefile_set = true;
            cout<< "Saving csv file: "<<*file_name<<endl;
        }
    }
    if( !savefile_set ){
        cout << "No savefile specified. Not saving results."<<endl;
        return false;
    }
    return true;
}

bool get_quietmode(int argc, char **argv)
{
    int i;
    bool quietmode = false;

    for (i=0; i<argc; i++){
        if(strcmp(argv[i],"-q")==0){
            quietmode = true;
        }
    }
    return quietmode;
}

bool get_sweepmode(int argc, char **argv)
{
    int i;
    bool sweepmode = false;

    for (i=0; i<argc; i++){
        if(strcmp(argv[i],"-sweep")==0){
            sweepmode = true;
        }
    }
    return sweepmode;
}

int get_replacement_policy(int argc, char **argv, string* replacement_policy)
{
    int i;
    bool line_size_set = false;

    for (i=0; i<argc; i++){
        if((strcmp(argv[i],"-replacementpolicy")==0) && (i< (argc+1)) ){
            if (strcmp (argv[i+1],"LRU") == 0 ||
                strcmp (argv[i+1],"NMRU") == 0 ||
                strcmp (argv[i+1],"random") == 0){
            *replacement_policy = argv[i+1];
            line_size_set = true;
            cout<< "Replacement policy set: "<<*replacement_policy<<endl;
            } else {
                cout<<"Error: Invalid replacement policy. Options are LRU,NMRU,random. Exiting."<<endl;
                return -1;
            }
        }
    }
    if( !line_size_set ){
        cout << "Replacement policy not specified. Using default LRU policy."<<endl;
        cout << "For direct mapped (ways == 1) replacement policy doesn't matter." <<endl;
        *replacement_policy = "LRU";
    }
    return 0;
}

unsigned int hexStrToInt (const char* str){
    size_t strLength = strlen(str);
    unsigned int result = 0;

    int i;
    unsigned int curr;
    for (i=strLength-1; i>=0; i--){
        if (str[i] == '0') curr = 0;
        else if (str[i] == '1') curr = 1;
        else if (str[i] == '2') curr = 2;
        else if (str[i] == '3') curr = 3;
        else if (str[i] == '4') curr = 4;
        else if (str[i] == '5') curr = 5;
        else if (str[i] == '6') curr = 6;
        else if (str[i] == '7') curr = 7;
        else if (str[i] == '8') curr = 8;
        else if (str[i] == '9') curr = 9;
        else if (str[i] == 'a' || str[i] == 'A') curr = 10;
        else if (str[i] == 'b' || str[i] == 'B') curr = 11;
        else if (str[i] == 'c' || str[i] == 'C') curr = 12;
        else if (str[i] == 'd' || str[i] == 'D') curr = 13;
        else if (str[i] == 'e' || str[i] == 'E') curr = 14;
        else if (str[i] == 'f' || str[i] == 'F') curr = 15;
        else return 0;

        result+= curr*pow(16,strLength-i-1);
    }

    return result;

}
