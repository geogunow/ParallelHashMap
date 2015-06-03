#ifndef __PARALLEL_HASH_MAP__
#define __PARALLEL_HASH_MAP__
#include<iostream>
#include<string>
#include<functional>

class fixed_hash_map
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

        fixed_hash_map(size_t M = 8);
        virtual ~fixed_hash_map();
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
        fixed_hash_map *_table;
        fixed_hash_map* volatile *_announce;
        size_t _threads;
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
