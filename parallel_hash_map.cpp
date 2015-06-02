#include"parallel_hash_map.h"

/*
    TODO: Constructor description
*/
parallel_hash_map::parallel_hash_map(size_t M)
{
    // allocate table
    _table.M = M;
    _table.N = 0;
    _table.buckets = new node*[_table.M]();

    // record the number of threads
    _threads = 1;

    // TODO: ensure that M is a power of 2
}

/*
    TODO: Destructor description
*/
parallel_hash_map::~parallel_hash_map()
{
    // FIXME
    delete[] _table.buckets;
} 

/*
    TODO: Insert description
*/
void parallel_hash_map::insert(std::string key, int value)
{

   
    // determine if resize necessary
    if(2*_table.N > _table.M)
        resize();

    // get hashes -- key_hash assumes M is a power of 2, uses fast modulus
    size_t key_hash = std::hash<std::string>()(key) & (_table.M-1);
    std::cout << "Attempting to insert at hash " << key_hash << std::endl;
    std::cout << "When the current map is:" << std::endl;
    print_buckets();
    size_t lock_hash = key_hash % _threads;

    // check to see if key already exisits in map
    if(contains(key))
        return;
 
    // create new node
    node *new_node = new node(key, value);

    // set lock
    
    // TODO: recheck if element exists
    
    // place element in linked list
    node **iter_node = &_table.buckets[key_hash];
    while(*iter_node != NULL)
        iter_node = &(*iter_node)->next;

    // place element in linked list
    *iter_node = new_node;
    _table.N += 1;

    // unset lock
    return;
}

/*
    TODO: Contains description
*/
bool parallel_hash_map::contains(std::string key)
{
    // get hash into table
    size_t key_hash = std::hash<std::string>()(key) & (_table.M-1);

    node *iter_node = _table.buckets[key_hash];
    while(iter_node != NULL)
    {
        //FIXME: THIS IS THE PROBLEM
        //std::cout << "Key = " << iter_node->key << std::endl;
        if(iter_node->key == key)
            return true;
        else
            iter_node = iter_node->next;
    }
    return false;
}

/*
    TODO: resize description
*/
void parallel_hash_map::resize()
{
    // TODO: acquire all locks first
    std::cout << "Resizing" << std::endl;

    // allocate new hash map
    parallel_hash_map new_map(2*_table.M);
    std::cout << "Allocation:" << std::endl;
    new_map.print_buckets();

    // fill map with key value pairs
    for(int i=0; i<_table.M; i++)
    {
        node *iter_node = _table.buckets[i];
        while(iter_node != NULL)
        {
            std::string key = iter_node->key;
            int value = iter_node->value;
            new_map.insert(key, value);
            iter_node = iter_node->next;
        }
    }

    // finally switch pointer
    std::cout << "New Map:" << std::endl;
    new_map.print_buckets();
    table* old_table = &_table;
    _table = new_map._table;
    
    std::cout << "Coppied Map:" << std::endl;
    print_buckets();
    
    // TODO: delete old table entries

    // TODO: release all locks
    return;
}

/*
    TODO: GetVal description
*/
int parallel_hash_map::getVal(std::string key)
{
    // TODO: optimize hashing
    size_t key_hash = std::hash<std::string>()(key) & (_table.M-1);

    node *iter_node = _table.buckets[key_hash];
    while(iter_node != NULL)
        if(iter_node->key == key)
            return iter_node->value;
        else
            iter_node = iter_node->next;
    
    throw 20;

    return 0;
}

/*
    Returns the number of entries in the map
*/
size_t parallel_hash_map::size()
{
    return _table.N;
}

/*
   Returns an array of the keys in the map
*/
std::string* parallel_hash_map::keys()
{
    // allocate array of strings
    std::string *keys = new std::string[_table.N];
    int ind = 0;
    for(int i=0; i<_table.M; i++)
    {
        node *iter_node = _table.buckets[i];
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
int* parallel_hash_map::values()
{
    // allocate array of strings
    int *values = new int[_table.N];
    int ind = 0;
    for(int i=0; i<_table.M; i++)
    {
        node *iter_node = _table.buckets[i];
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
void parallel_hash_map::print_buckets()
{
    for(int i=0; i<_table.M; i++)
    {
        if(_table.buckets[i] == NULL)
            std::cout << i << " -> NULL" << std::endl;
        else
            std::cout << i << " -> " << _table.buckets[i] << std::endl;
    }
}
