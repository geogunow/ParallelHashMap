#ifndef __PARALLEL_HASH_MAP__
#define __PARALLEL_HASH_MAP__
#include<iostream>
#include<functional>
#ifdef OPENMP
#include<omp.h>
#endif

template <class K, class V>
class fixed_hash_map
{
    struct node
    {
        node(K k_in, V v_in) : next(NULL), key(k_in), value(v_in){}
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

template <class K, class V>
class parallel_hash_map
{
    // padded pointer to hash table to avoid false sharing
    struct paddedPointer
    {
        volatile long pad_L1;
        volatile long pad_L2;
        volatile long pad_L3;
        volatile long pad_L4;
        volatile long pad_L5;
        volatile long pad_L7;
        volatile long pad_L8;
        fixed_hash_map<K,V>* volatile value;
        volatile long pad_R1;
        volatile long pad_R2;
        volatile long pad_R3;
        volatile long pad_R4;
        volatile long pad_R5;
        volatile long pad_R6;
        volatile long pad_R7;
        volatile long pad_R8;
    };
    private:
        fixed_hash_map<K,V> *_table;
        paddedPointer *_announce;
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

/**
 * @brief Constructor initializes fixed-size table of buckets filled with empty
 *          linked lists.
 * @details The constructor initializes a fixed-size hash map with the size
 *          as an input parameter. If no size is given the default size (64)
 *          is used. Buckets are filled with empty linked lists presented as
 *          NULL pointers.
 * @param M size of fixed hash map
 */
template <class K, class V>
fixed_hash_map<K,V>::fixed_hash_map(size_t M)
{
    // allocate table
    _M = M;
    _N = 0;
    _buckets = new node*[_M]();
}

/**
 * @breif Destructor deletes all nodes in the linked lists associate with each
 *          bucket in the fixed-size table and their pointers.
 */
template <class K, class V>
fixed_hash_map<K,V>::~fixed_hash_map()
{
    // for each bucket, scan through linked list and delete all nodes
    for(size_t i=0; i<_M; i++)
    {
        node *iter_node = _buckets[i];
        while(iter_node != NULL)
        {
            node *next_node = iter_node->next;
            delete iter_node;
            iter_node = next_node;
        }
    } 

    // delete all buckets (now pointers to empty linked lists)
    delete[] _buckets;
} 

/**
 * @brief Determine whether the fixed-size table contains a given key
 * @details The linked list in the bucket associated with the key is searched
 *             to determine whether the key is present.
 * @param key to be searched
 * @return boolean value referring to whether the key is contained in the map
 */
template <class K, class V>
bool fixed_hash_map<K,V>::contains(K key)
{
    // get hash into table assuming M is a power of 2, using fast modulus
    size_t key_hash = std::hash<K>()(key) & (_M-1);

    // search corresponding bucket for key
    node *iter_node = _buckets[key_hash];
    while(iter_node != NULL)
    {
        if(iter_node->key == key)
            return true;
        else
            iter_node = iter_node->next;
    }
    return false;
}

/**
 * @brief Determine the value associated with a given key in the fixed-size
 *          table.
 * @details The linked list in the bucket associated with the key is searched
 *          and once the key is found, the corresponding value is returned.
 *          An exception is thrown if the key is not present in the map.
 * @param key whose corresponding value is desired
 * @return value associated with the given key
 */
template <class K, class V>
V fixed_hash_map<K,V>::at(K key)
{
    // get hash into table assuming M is a power of 2, using fast modulus
    size_t key_hash = std::hash<K>()(key) & (_M-1);

    // search bucket for key and return the corresponding value if found
    node *iter_node = _buckets[key_hash];
    while(iter_node != NULL)
        if(iter_node->key == key)
            return iter_node->value;
        else
            iter_node = iter_node->next;
    
    // after the bucket has been completely searched without finding the key,
    // throw an exception
    throw 20;

    return -1;
}


/**
 * @brief Insert a key/value pair into the fixed-size table.
 * @details The specified key value pair is inserted into the fixed-size table.
 *          If the key already exists in the table, the pair is not inserted
 *          and the function returns.
 * @param key of the key/value pair to be inserted
 * @param value of the key/value pair to be inserted
 */
template <class K, class V>
void fixed_hash_map<K,V>::insert(K key, V value)
{
    // get hash into table assuming M is a power of 2, using fast modulus
    size_t key_hash = std::hash<K>()(key) & (_M-1);

    // check to see if key already exisits in map
    if(contains(key))
        return;
 
    // create new node
    node *new_node = new node(key, value);

    // find where to place element in linked list
    node **iter_node = &_buckets[key_hash];
    while(*iter_node != NULL)
        iter_node = &(*iter_node)->next;

    // place element in linked list
    *iter_node = new_node;
    
    // increment counter
    #pragma omp atomic
    _N++;

    return;
}

/**
 * @brief Returns the number of key/value pairs in the fixed-size table
 * @return number of key/value pairs in the map
 */
template <class K, class V>
size_t fixed_hash_map<K,V>::size()
{
    return _N;
}

/**
 * @brief Returns the number of buckets in the fixed-size table
 * @return number of buckets in the map
 */
template <class K, class V>
size_t fixed_hash_map<K,V>::bucket_count()
{
    return _M;
}

/**
 * @brief Returns an array of the keys in the fixed-size table
 * @details All buckets are scanned in order to form a list of all keys
 *          present in the table and then the list is returned
 * @return an array of keys in the map whose length is the number of key/value
 *          pairs in the table.
*/
template <class K, class V>
K* fixed_hash_map<K,V>::keys()
{
    // allocate array of keys
    K *key_list = new K[_N];
    size_t ind = 0;
    for(size_t i=0; i<_M; i++)
    {
        node *iter_node = _buckets[i];
        while(iter_node != NULL)
        {
            key_list[ind] = iter_node->key;
            iter_node = iter_node->next;
            ind++;
        }
    }
    return key_list;
}

/**
 * @brief Returns an array of the values in the fixed-size table
 * @details All buckets are scanned in order to form a list of all values
 *          present in the table and then the list is returned
 * @return an array of values in the map whose length is the number of 
 *          key/value pairs in the table.
*/
template <class K, class V>
V* fixed_hash_map<K,V>::values()
{
    // allocate array of keys
    V *values = new V[_N];
    size_t ind = 0;
    for(size_t i=0; i<_M; i++)
    {
        node *iter_node = _buckets[i];
        while(iter_node != NULL)
        {
            values[ind] = iter_node->value;
            iter_node = iter_node->next;
            ind++;
        }
    }
    return values;
}

/**
 * @brief Prints the contents of each bucket to the screen
 * @details All buckets are scanned and the contents of the buckets are
 *          printed, which are pointers to linked lists. If the pointer is NULL
 *          suggesting that the linked list is empty, NULL is printed to the
 *          screen.
 */
template <class K, class V>
void fixed_hash_map<K,V>::print_buckets()
{
    for(size_t i=0; i<_M; i++)
    {
        if(_buckets[i] == NULL)
            std::cout << i << " -> NULL" << std::endl;
        else
            std::cout << i << " -> " << _buckets[i] << std::endl;
    }
}


/*
    TODO: parallel hash map description
*/

/**
 * @brief Constructor for generates initial underlying table as a fixed-sized 
 *          hash map and intializes concurrency structures.
 */
template <class K, class V>
parallel_hash_map<K,V>::parallel_hash_map(size_t M, size_t L)
{
    //TODO: check that L is a power of 2 (round up)

    // ensure that L is less than or equal to M
    if(L > M) M = L;

    // allocate table
    _table = new fixed_hash_map<K,V>(M);

    // get number of threads and create announce array
    _num_threads = 1;
    #ifdef OPENMP
    _num_threads = omp_get_max_threads();
    _num_locks = L;
    _locks = new omp_lock_t[_num_locks];
    for(size_t i=0; i<_num_locks; i++)
        omp_init_lock(&_locks[i]);
    #endif

    _announce = new paddedPointer[_num_threads];
}

/**
 * @breif Destructor frees memory associated with fixed-sized hash map and
 *          concurrency structures.
 */
template <class K, class V>
parallel_hash_map<K,V>::~parallel_hash_map()
{
    delete _table;
    #ifdef OPENMP  
    delete[] _locks;
    #endif
    delete[] _announce;
}

/**
 * @brief Determine whether the parallel hash map contains a given key
 * @details First the thread accessing the table announces its presence and
 *          which table it is reading. Then the linked list in the bucket 
 *          associated with the key is searched without setting any locks
 *          to determine whether the key is present. When the thread has
 *          finished accessing the table, the announcement is reset to NULL.
 *          The announcement ensures that the data in the map is not freed
 *          during a resize until all threads have finished accessing the map.
 * @param key to be searched
 * @return boolean value referring to whether the key is contained in the map
 */
template <class K, class V>
bool parallel_hash_map<K,V>::contains(K key)
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched, ensure consistency
    fixed_hash_map<K,V> *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid].value = table_ptr;
    } while(table_ptr != _table);

    // see if current table contians the thread
    bool present = table_ptr->contains(key);
    
    // reset table announcement to not searching
    _announce[tid].value = NULL;
    
    return present;
}

/**
 * @brief Determine the value associated with a given key.
 * @details This function follows the same algorithm as <contains> except that
 *          the value associated with the searched key is returned.
 *          First the thread accessing the table announces its presence and
 *          which table it is reading. Then the linked list in the bucket 
 *          associated with the key is searched without setting any locks
 *          to determine the associated value. An exception is thrown if the 
 *          key is not found. When the thread has finished accessing the table, 
 *          the announcement is reset to NULL. The announcement ensures that 
 *          the data in the map is not freed during a resize until all threads 
 *          have finished accessing the map.
 * @param key to be searched
 * @return value associated with the key
 */
template <class K, class V>
V parallel_hash_map<K,V>::at(K key)
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map<K,V> *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid].vlaue = table_ptr;
    } while(table_ptr != _table);

    // see if current table contians the thread
    V value = table_ptr->at(key);
    
    // reset table announcement to not searching
    _announce[tid].value = NULL;
    
    return value;
}

/**
 * @brief Insert a given key/value pair into the parallel hash map.
 * @details First the underlying table is checked to determine if a resize
 *          should be conducted. Then, the table is checked to see if it
 *          already contains the key. If so, the key/value pair is not inserted
 *          and the function returns. Otherwise, the lock of the associated
 *          bucket is acquired and the key/value pair is added to the bucket.
 * @param key of the key/value pair to be inserted
 * @param value of the key/value pair to be inserted
 */
template <class K, class V>
void parallel_hash_map<K,V>::insert(K key, V value)
{
    // check if resize needed
    if(2*_table->size() > _table->bucket_count())
        resize();

    // check to see if key is already contained in the table
    if(contains(key))
        return;

    // get lock hash
    #ifdef OPENMP
    size_t lock_hash = std::hash<K>()(key) & (_num_locks - 1);

    // acquire lock
    omp_set_lock(&_locks[lock_hash]);
    #endif

    // insert value
    _table->insert(key, value);

    // release lock
    #ifdef OPENMP
    omp_unset_lock(&_locks[lock_hash]);
    #endif
   
    return;
}

/*
    TODO: Resize description
*/
template <class K, class V>
void parallel_hash_map<K,V>::resize()
{
    // acquire all locks in order
    #ifdef OPENMP
    for(size_t i=0; i<_num_locks; i++)
        omp_set_lock(&_locks[i]);
    #endif 

    // recheck if resize needed
    if(2*_table->size() < _table->bucket_count())
    {
        // release locks
        #ifdef OPENMP
        for(size_t i=0; i<_num_locks; i++)
            omp_unset_lock(&_locks[i]);
        #endif 

        return;
    }

    // allocate new hash map of double the size
    fixed_hash_map<K,V> *new_map = 
        new fixed_hash_map<K,V>(2*_table->bucket_count());

    // get keys, values, and number of elements
    K *key_list = _table->keys();
    V *value_list = _table->values();

    // insert key/value pairs into new hash map
    for(size_t i=0; i<_table->size(); i++)
        new_map->insert(key_list[i], value_list[i]);

    // save pointer of old table
    fixed_hash_map<K,V> *old_table = _table;

    // reassign pointer
    _table = new_map;

    // release all locks
    #ifdef OPENMP
    for(size_t i=0; i<_num_locks; i++)
        omp_unset_lock(&_locks[i]);
    #endif

    // delete key and value list
    delete[] key_list;
    delete[] value_list;

    // wait for all threads to stop reading from the old table
    for(size_t i=0; i<_num_threads; i++)
        while(_announce[i].value == old_table) {};

    // free memory associated with old table
    delete old_table;

    return;
}

/**
 * @brief Returns the number of key/value pairs in the underlying table
 * @return number of key/value pairs in the map
 */
template <class K, class V>
size_t parallel_hash_map<K,V>::size()
{
    return _table->size();
}

/**
 * @brief Returns the number of buckets in the underlying table
 * @return number of buckets in the map
 */
template <class K, class V>
size_t parallel_hash_map<K,V>::bucket_count()
{
    return _table->bucket_count();
}

/**
 * @brief Returns an array of the keys in the underlying table
 * @details All buckets are scanned in order to form a list of all keys
 *          present in the table and then the list is returned. Threads
 *          announce their presence to ensure table memory is not freed
 *          during access.
 * @return an array of keys in the map whose length is the number of key/value
 *          pairs in the table.
 */
template <class K, class V>
K* parallel_hash_map<K,V>::keys()
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map<K,V> *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid].value = table_ptr;
    } while(table_ptr != _table);

    // get key list
    K* key_list = _table->keys();

    // reset table announcement to not searching
    _announce[tid].value = NULL;

    return key_list;
}

/**
 * @brief Returns an array of the values in the underlying table
 * @details All buckets are scanned in order to form a list of all values
 *          present in the table and then the list is returned. Threads
 *          announce their presence to ensure table memory is not freed
 *          during access.
 * @return an array of values in the map whose length is the number of key/value
 *          pairs in the table.
 */
template <class K, class V>
V* parallel_hash_map<K,V>::values()
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map<K,V> *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid].value = table_ptr;
    } while(table_ptr != _table);

    // get value list
    V* value_list = _table->values();
    
    // reset table announcement to not searching
    _announce[tid].value = NULL;

    return value_list;
}

/**
 * @brief Prints the contents of each bucket to the screen
 * @details All buckets are scanned and the contents of the buckets are
 *          printed, which are pointers to linked lists. If the pointer is NULL
 *          suggesting that the linked list is empty, NULL is printed to the
 *          screen. Threads announce their presence to ensure table memory is
 *          not freed during access.
 */
template <class K, class V>
void parallel_hash_map<K,V>::print_buckets()
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map<K,V> *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid].value = table_ptr;
    } while(table_ptr != _table);

    // print buckets
    _table->print_buckets();

    // reset table announcement to not searching
    _announce[tid].value = NULL;
    
    return;
}


#endif
