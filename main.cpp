#include"parallel_hash_map.h"
#include<time.h>

int main()
{
    closed_hash_map X = closed_hash_map();

    std::cout << "This should be false ... " << std::endl;
    std::cout << X.contains("hello") << std::endl;
    std::cout << "This should be true ... " << std::endl;
    X.insert("hello", 5);
    std::cout << X.contains("hello") << std::endl;
    std::cout << "This should be 5 ... " << std::endl;
    std::cout << X.at("hello") << std:: endl;
    X.insert("hello", 3);
    X.insert("goodbye", 4);
    std::cout << "This should be 4 ... " << std::endl;
    std::cout << X.at("goodbye") << std:: endl;

    // timing studies
    clock_t t1, t2;
    t1 = clock();
    
    // setup problem
    long num = 1;
    long a = 1664525;
    long c = 1013904223;
    long m = 0x01 << 31;
    long len = 0x01 << 20;
    std::cout << "Len = " << len << std::endl;
    long prime = 997;
    std::string base_string("String_");
    
    for(int i=0; i<len; i++)
    {
        // form key name    
        num = (a*num + c) % m;
        char ext = (char) (num % prime);
        std::string key = base_string;
        key += ext;
        
        X.insert(key, i);
    }
    for(int i=0; i<5*len; i++)
    {
        std::string key = base_string;
        key += (char) i;
        bool ans = X.contains(key);
    }

    t2 = clock();
    float diff = (float) t2 - (float) t1;
    std::cout << "Elapsed time = " << diff << std::endl;
    std::cout << "Size = " << X.size() << std::endl;
    return 0;
}
