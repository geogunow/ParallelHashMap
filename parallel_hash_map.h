#ifndef __PARALLEL_HASH_MAP__
#define __PARALLEL_HASH_MAP__
#include<iostream>
#include<string>
#include<functional>

class parallel_hash_map
{
    struct node
    {
        node(std::string k, int v) : next(NULL), key(k), value(v){}
        std::string key;
        int value;
        node *next;
    };
 
    private:
        size_t _M;        // table size
        size_t _N;        // number of elements present in table
        node **_table;    // table of values stored in nodes
        size_t _threads;  // number of threads accessing the table
        void resize();    // function that resizes hash table to twice the size
    public:

        parallel_hash_map(size_t M = 64);
        virtual ~parallel_hash_map();
        bool contains(std::string key);
        int getVal(std::string key);
        void insert(std::string str, int val);
        size_t size();
        std::string* keys();
        int* values();
        void print_buckets();
};

#endif
