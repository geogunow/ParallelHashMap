#ifndef __PARALLEL_HASH_MAP__
#define __PARALLEL_HASH_MAP__
#include<iostream>
#include<string>
#include<functional>

class closed_hash_map
{
    struct node
    {
        node(std::string k, int v) : next(NULL), key(k), value(v){}
        std::string key;
        int value;
        node *next;
    };

    private:
        size_t _M;          // table size
        size_t _N;          // number of elements present in table
        node ** _buckets;   // buckets of values stored in nodes

    public:

        closed_hash_map(size_t M = 8);
        virtual ~closed_hash_map();
        bool contains(std::string key);
        int at(std::string key);
        void insert(std::string str, int value);
        size_t size();
        size_t bucket_count();
        std::string* keys();
        int* values();
        void print_buckets();
};

class parallel_hash_map
{
    private:
        closed_hash_map *_table;
        void resize();

    public:
        parallel_hash_map(size_t M = 8);
        virtual ~parallel_hash_map();
        bool contains(std::string key);
        int at(std::string key);
        void insert(std::string str, int value);
        size_t size();
        size_t bucket_count();
        std::string* keys();
        int* values();
        void print_buckets();
};
#endif
