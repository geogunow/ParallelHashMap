#include"parallel_hash_map.h"

/**
 * @brief Constructor initializes fixed-size table of buckets filled with empty
 *          linked lists.
 * @details The constructor initializes a fixed-size hash map with the size
 *          as an input parameter. If no size is given the default size (64)
 *          is used. Buckets are filled with empty linked lists presented as
 *          NULL pointers.
 * @param M size of fixed hash map
 */
fixed_hash_map::fixed_hash_map(size_t M)
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
fixed_hash_map::~fixed_hash_map()
{
    // for each bucket, scan through linked list and delete all nodes
    for(int i=0; i<_M; i++)
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
bool fixed_hash_map::contains(std::string key)
{
    // get hash into table assuming M is a power of 2, using fast modulus
    size_t key_hash = std::hash<std::string>()(key) & (_M-1);

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
int fixed_hash_map::at(std::string key)
{
    // get hash into table assuming M is a power of 2, using fast modulus
    size_t key_hash = std::hash<std::string>()(key) & (_M-1);

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
void fixed_hash_map::insert(std::string key, int value)
{
    // get hash into table assuming M is a power of 2, using fast modulus
    size_t key_hash = std::hash<std::string>()(key) & (_M-1);

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
    _N += 1;

    return;
}

/**
 * @brief Returns the number of key/value pairs in the fixed-size table
 * @return number of key/value pairs in the map
 */
size_t fixed_hash_map::size()
{
    return _N;
}

/**
 * @brief Returns the number of buckets in the fixed-size table
 * @return number of buckets in the map
 */
size_t fixed_hash_map::bucket_count()
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
std::string* fixed_hash_map::keys()
{
    // allocate array of strings
    std::string *keys = new std::string[_N];
    size_t ind = 0;
    for(int i=0; i<_M; i++)
    {
        node *iter_node = _buckets[i];
        while(iter_node != NULL)
        {
            keys[ind] = iter_node->key;
            iter_node = iter_node->next;
            ind++;
        }
    }
    return keys;
}

/**
 * @brief Returns an array of the values in the fixed-size table
 * @details All buckets are scanned in order to form a list of all values
 *          present in the table and then the list is returned
 * @return an array of values in the map whose length is the number of 
 *          key/value pairs in the table.
*/

int* fixed_hash_map::values()
{
    // allocate array of strings
    int *values = new int[_N];
    size_t ind = 0;
    for(int i=0; i<_M; i++)
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
void fixed_hash_map::print_buckets()
{
    for(int i=0; i<_M; i++)
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
parallel_hash_map::parallel_hash_map(size_t M, size_t L)
{
    //TODO: check that L is a power of 2 (round up)

    // ensure that L is less than or equal to M
    if(L > M) M = L;

    // allocate table
    _table = new fixed_hash_map(M);

    // get number of threads and create announce array
    _num_threads = 1;
    #ifdef OPENMP
    _num_threads = omp_get_num_threads();
    _num_locks = L;
    _locks = new omp_lock_t[_num_locks];
    #endif

    _announce = new fixed_hash_map* volatile[_num_threads];
}

/**
 * @breif Destructor frees memory associated with fixed-sized hash map and
 *          concurrency structures.
 */
parallel_hash_map::~parallel_hash_map()
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
bool parallel_hash_map::contains(std::string key)
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched, ensure consistency
    fixed_hash_map *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid] = table_ptr;
    } while(table_ptr != _table);

    // see if current table contians the thread
    bool present = table_ptr->contains(key);
    
    // reset table announcement to not searching
    _announce[tid] = NULL;
    
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
int parallel_hash_map::at(std::string key)
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid] = table_ptr;
    } while(table_ptr != _table);

    // see if current table contians the thread
    int value = table_ptr->at(key);
    
    // reset table announcement to not searching
    _announce[tid] = NULL;
    
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
void parallel_hash_map::insert(std::string key, int value)
{
    // check if resize needed
    if(2*_table->size() > _table->bucket_count())
        resize();

    // check to see if key is already contained in the table
    if(contains(key))
        return;

    // get lock hash
    #ifdef OPENMP
    size_t lock_hash = std::hash<std::string>()(key) & (_num_locks - 1);

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
void parallel_hash_map::resize()
{
    // acquire all locks in order
    #ifdef OPENMP
    for(size_t i=0; i<_num_locks; i++)
        omp_set_lock(&_locks[i]);
    #endif 

    // recheck if resize needed
    if(2*_table->size() < _table->bucket_count())
        return;

    // allocate new hash map of double the size
    fixed_hash_map *new_map = new fixed_hash_map(2*_table->bucket_count());

    // get keys, values, and number of elements
    std::string *key_list = _table->keys();
    int *value_list = _table->values();
    int N = _table->size();

    // insert key/value pairs into new hash map
    for(int i=0; i<N; i++)
        new_map->insert(key_list[i], value_list[i]);

    // save pointer of old table
    fixed_hash_map *old_table = _table;

    // reassign pointer
    _table = new_map;

    // release all locks
    #ifdef OPENMP
    for(size_t i=0; i<_num_locks; i++)
        omp_unset_lock(&_locks[i]);
    #endif

    // wait for all threads to stop reading from the old table
    for(int i=0; i<_num_threads; i++)
        while(_announce[i] == old_table) {};

    // free memory associated with old table
    delete old_table;

    return;
}

/**
 * @brief Returns the number of key/value pairs in the underlying table
 * @return number of key/value pairs in the map
 */
size_t parallel_hash_map::size()
{
    return _table->size();
}

/**
 * @brief Returns the number of buckets in the underlying table
 * @return number of buckets in the map
 */
size_t parallel_hash_map::bucket_count()
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
std::string* parallel_hash_map::keys()
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid] = table_ptr;
    } while(table_ptr != _table);

    // get key list
    std::string* key_list = _table->keys();

    // reset table announcement to not searching
    _announce[tid] = NULL;

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
int* parallel_hash_map::values()
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid] = table_ptr;
    } while(table_ptr != _table);

    // get value list
    int* value_list = _table->values();
    
    // reset table announcement to not searching
    _announce[tid] = NULL;

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
void parallel_hash_map::print_buckets()
{
    // get thread ID
    size_t tid = 0;
    #ifdef OPENMP
    tid = omp_get_thread_num();
    #endif

    // get pointer to table, announce it will be searched
    fixed_hash_map *table_ptr;
    do{
        table_ptr = _table;
        _announce[tid] = table_ptr;
    } while(table_ptr != _table);

    // print buckets
    _table->print_buckets();

    // reset table announcement to not searching
    _announce[tid] = NULL;
    
    return;
}

