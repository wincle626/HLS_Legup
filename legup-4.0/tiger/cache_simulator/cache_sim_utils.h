#ifndef CACHE_SIM_UTILS_H
#define CACHE_SIM_UTILS_H

#include <string>

#define MAX_WAYS 256

int get_line_size (int argc, char **argv);
int get_file_name(int argc, char **argv, std::string* file_name);
unsigned int hexStrToInt (const char* str);
int get_cache_size (int argc, char **argv);
int get_ways_per_set (int argc, char **argv);
int get_replacement_policy(int argc, char **argv, std::string* replacement_policy);
bool get_quietmode(int argc, char **argv);
int get_prefetch (int argc, char **argv);
bool get_sweepmode(int argc, char **argv);
bool get_savefile(int argc, char **argv, std::string* file_name);

// Keep track of which is most recently used by shifting the array.
// element 0 is the most recently used.
typedef struct {
    unsigned int ways[MAX_WAYS];
    bool valid_bits[MAX_WAYS];
} cache_set;


#endif
