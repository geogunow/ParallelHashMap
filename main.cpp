#include"parallel_hash_map.h"
#include<time.h>
#include<string>

int main()
{
    // set up threads
    #ifdef OPENMP
    size_t max_threads = omp_get_num_procs();
    std::cout << "Requesting " << max_threads << " threads\n";
    max_threads = 1;
    omp_set_num_threads(max_threads);
    #endif

    // initialize hash map
    parallel_hash_map<char,int> X;// = parallel_hash_map<std::string,int>();

    std::cout << "This should be false ... " << std::endl;
    std::cout << X.contains('A') << std::endl;
    std::cout << "This should be true ... " << std::endl;
    X.insert('B', 5);
    std::cout << X.contains('B') << std::endl;
    std::cout << "This should be 5 ... " << std::endl;
    std::cout << X.at('B') << std:: endl;
    X.insert('C', 3);
    X.insert('D', 4);
    std::cout << "This should be 3 ... " << std::endl;
    std::cout << X.at('C') << std:: endl;

    // timing studies
    //clock_t t1, t2;
    //t1 = clock();
    double t1 = omp_get_wtime();

    // setup problem
    long num = 1;
    long a = 1664525;
    long c = 1013904223;
    long m = 0x01 << 31;
    long len = 0x01 << 20;
    std::cout << "Len = " << len << std::endl;
    long prime = 997;
    char base_char = 'a';

    std::cout << "Starting inserts..." << std::endl;
    #pragma omp parallel default(none) \
    shared(X, m, prime, a, c, len, base_char) private(num)
    {
        num = prime / (omp_get_thread_num() + 1) + 1;
        for(int i=0; i<len; i++)
        {
            // form key name    
            num = (a*num + c) % m;
            char ext = (char) (num % prime);
            char key = base_char;
            key += ext;
            
            X.insert(key, i);
        }
    }

    #pragma omp parallel for default(none) \
    shared(X, len, base_char)
    for(int i=0; i<5*len; i++)
    {
        char key = base_char;
        key += (char) i;
        bool ans = X.contains(key);
    }

    //t2 = clock();
    double t2 = omp_get_wtime();
    float diff = (float) t2 - (float) t1;
    std::cout << "Elapsed time = " << diff << std::endl;
    std::cout << "Size = " << X.size() << std::endl;
    
    return 0;
}
