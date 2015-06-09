#include"parallel_hash_map.h"
#include<time.h>
#include<string>

int main()
{
    // set up threads
    #ifdef OPENMP
    size_t max_threads = omp_get_num_procs();
    std::cout << "Requesting " << max_threads << " threads\n";
    omp_set_num_threads(max_threads);
    #endif

    // initialize hash map
    parallel_hash_map<char,int> X;

    // timing studies
    double t1, t2;
    #ifdef OPENMP
    t1 = omp_get_wtime();
    #else
    t1 = (double) clock() / 1e6;
    #endif

    // setup problem
    long num = 1;
    long a = 1664525;
    long c = 1013904223;
    long m = 0x01 << 31;
    long len = 0x01 << 24;
    long prime = 997;

    std::cout << "Starting inserts..." << std::endl;
    #pragma omp parallel default(none) \
    shared(X, m, prime, a, c, len) private(num)
    {
        #ifdef OPENMP
        num = prime / (omp_get_thread_num() + 1) + 1;
        #endif
        #pragma omp for
        for(int i=0; i<len; i++)
        {
            // form key name    
            num = (a*num + c) % m;
            
            X.insert(num, i);
        }
    }

    int sum = 0;
    #pragma omp parallel for default(none) \
    shared(X, len) schedule(dynamic,100) \
    reduction(+:sum)
    for(int i=0; i<5*len; i++)
    {
        bool ans = X.contains(i);
        sum += (int) ans;
    }

    #ifdef OPENMP
    t2 = omp_get_wtime();
    #else
    t2 = (double) clock() / 1e6;
    #endif
    float diff = (float) t2 - (float) t1;
    std::cout << "Elapsed time = " << diff << std::endl;
    std::cout << "Size = " << X.size() << std::endl;
    
    return 0;
}
