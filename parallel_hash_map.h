#ifndef __PARALLEL_HASH_MAP__
#define __PARALLEL_HASH_MAP__
#include<iostream>
#include<string>
#include<functional>
#ifdef OPENMP
#include<omp.h>
#endif

template <typename K, typename V>
class fixed_hash_map
{
    struct node
    {
        node(K k, V v) : next(NULL), key(k), value(v){}
        K key;
        V value;
        node *next;
    };

    private:
        size_t _M;          // table size
        size_t _N;          // number of elements present in table
        node ** _buckets;   // buckets of values stored in nodes

    public:

        fixed_hash_map(size_t M = 64);
        virtual ~fixed_hash_map();
        bool contains(K key);
        V at(K key);
        void insert(K key, V value);
        size_t size();
        size_t bucket_count();
        K* keys();
        V* values();
        void print_buckets();
};

template <typename K, typename V>
class parallel_hash_map
{
    private:
        fixed_hash_map<K,V> *_table;
        fixed_hash_map<K,V>* volatile *_announce;
        size_t _num_threads;
        size_t _N;
        #ifdef OPENMP
        omp_lock_t * _locks;
        size_t _num_locks;
        #endif
        void resize();

    public:
        parallel_hash_map(size_t M = 64, size_t L = 64);
        virtual ~parallel_hash_map();
        bool contains(K key);
        V at(K key);
        void insert(K key, V value);
        size_t size();
        size_t bucket_count();
        K* keys();
        V* values();
        void print_buckets();
};
#endif
