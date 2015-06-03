#include"parallel_hash_map.h"

/*
    TODO: Constructor description
*/
closed_hash_map::closed_hash_map(size_t M)
{
    // allocate table
    _M = M;
    _N = 0;
    _buckets = new node*[_M]();
}

/*
    TODO: Destructor description
*/
closed_hash_map::~closed_hash_map()
{
    // FIXME
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
        
    delete[] _buckets;
} 

/*
    TODO: Contains description
*/
bool closed_hash_map::contains(std::string key)
{
    // get hash into table
    size_t key_hash = std::hash<std::string>()(key) & (_M-1);

    node *iter_node = _buckets[key_hash];
    while(iter_node != NULL)
    {
        //std::cout << "Key = " << iter_node->key << std::endl;
        if(iter_node->key == key)
            return true;
        else
            iter_node = iter_node->next;
    }
    return false;
}


/*
    TODO: GetVal description
*/
int closed_hash_map::at(std::string key)
{
    // TODO: optimize hashing
    size_t key_hash = std::hash<std::string>()(key) & (_M-1);

    node *iter_node = _buckets[key_hash];
    while(iter_node != NULL)
        if(iter_node->key == key)
            return iter_node->value;
        else
            iter_node = iter_node->next;
    
    throw 20;

    return 0;
}


/*
    TODO: Insert description
*/
void closed_hash_map::insert(std::string key, int value)
{
    // get hashes -- key_hash assumes M is a power of 2, uses fast modulus
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

    // unset lock
    return;
}

/*
    Returns the number of entries in the map
*/
size_t closed_hash_map::size()
{
    return _N;
}

/*
    Returns the number of buckets in the map
*/
size_t closed_hash_map::bucket_count()
{
    return _M;
}

/*
   Returns an array of the keys in the map
*/
std::string* closed_hash_map::keys()
{
    // allocate array of strings
    std::string *keys = new std::string[_N];
    int ind = 0;
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

/*
   Returns an array of the values in the map
*/
int* closed_hash_map::values()
{
    // allocate array of strings
    int *values = new int[_N];
    int ind = 0;
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

// TODO: Delete
void closed_hash_map::print_buckets()
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

/*
    TODO: Constructor description
*/
parallel_hash_map::parallel_hash_map(size_t M)
{
    // allocate table
    _table = new closed_hash_map(M);
}

/*
    TODO: Destructor description
*/
parallel_hash_map::~parallel_hash_map()
{
    delete _table;
}

/*
    TODO: Contains description
*/
bool parallel_hash_map::contains(std::string key)
{ 
    return _table->contains(key);
}


/*
    TODO: GetVal description
*/
int parallel_hash_map::at(std::string key)
{
    return _table->at(key);
}

/*
    TODO: Insert description
*/
void parallel_hash_map::insert(std::string key, int value)
{
    // check if resize needed
    if(2*_table->size() > _table->bucket_count())
        resize();

    // TODO: get lock

    // insert value
    _table->insert(key, value);

    // TODO: unlock
    
    return;
}

/*
    TODO: Resize description
*/
void parallel_hash_map::resize()
{
    // allocate new hash map of double the size
    closed_hash_map *new_map = new closed_hash_map(2*_table->bucket_count());

    // TODO: acquire all locks sequentially

    // get keys, values, and number of elements
    std::string *key_list = _table->keys();
    int *value_list = _table->values();
    int N = _table->size();

    // insert key/value pairs into new hash map
    for(int i=0; i<N; i++)
        new_map->insert(key_list[i], value_list[i]);

    // reassign pointer
    _table = new_map;

    // TODO: release all locks

    return;
}

/*
    Returns the number of entries in the map
*/
size_t parallel_hash_map::size()
{
    return _table->size();
}

/*
    Returns the number of buckets in the map
*/
size_t parallel_hash_map::bucket_count()
{
    return _table->bucket_count();
}

/*
   Returns an array of the keys in the map
*/
std::string* parallel_hash_map::keys()
{
    return _table->keys();
}

/*
   Returns an array of the values in the map
*/
int* parallel_hash_map::values()
{
    return _table->values();
}

/*
   Prints contents of buckets in the map
*/
void parallel_hash_map::print_buckets()
{
    _table->print_buckets();
    return;
}

