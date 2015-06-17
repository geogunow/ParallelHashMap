#include"parallel_hash_map.h"
#include<time.h>
#include<string>

struct hamm{
    int x1;
    int x2;
};

int main()
{
    // set up threads
    #ifdef OPENMP
    size_t max_threads = omp_get_num_procs();
    std::cout << "Requesting " << max_threads << " threads\n";
    omp_set_num_threads(max_threads);
    #endif

    // initialize hash map
    parallel_hash_map<long,hamm*> X;

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
    //long prime = 997;
    long prime = 194;

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
            hamm *h = new hamm;
            int haha = X.insert_and_get_count(num%prime, h);
            h->x1 = i;
            h->x2 = haha;
        }
    }

    int sum = 0;
    #pragma omp parallel for default(none) \
    shared(X, len) schedule(dynamic,100) \
    reduction(+:sum)
    for(int i=0; i<5*len; i++)
    {
        long key = (long) i;
        int ans = (int) X.contains(key);
        sum += ans;
    }

    #ifdef OPENMP
    t2 = omp_get_wtime();
    #else
    t2 = (double) clock() / 1e6;
    #endif
    float diff = (float) t2 - (float) t1;
    std::cout << "Elapsed time = " << diff << std::endl;
    std::cout << "Size = " << X.size() << std::endl;
  
    hamm** value_list = X.values();
    for(int i=0; i < X.size(); i++)
    {
        std::cout << value_list[i]->x2 << std::endl;
    }



    return 0;
}
