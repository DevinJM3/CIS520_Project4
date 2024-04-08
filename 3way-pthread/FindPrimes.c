#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    unsigned whos_turn;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} shared_data;

typedef struct {
    unsigned val;
    shared_data *sd; // will point to the one and only instance of `shared_data`
} work_order;

#define SEARCH_RANGE (1000ULL)

void *findPrimes(void *arg) {
    work_order *wo = arg;

    uintmax_t primes[SEARCH_RANGE]; // to store the found primes
    int found_count = 0;

    for (uintmax_t i = wo->val*SEARCH_RANGE+1; i <= (wo->val+1)*SEARCH_RANGE; i += 2) {
        bool isPrime = true;
        for (uintmax_t j = 3; j < i; j += 2) {
            if (i % j == 0) {    // note: both i and j are odd
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            primes[found_count++] = i;
        }
    }
    if(wo->val == 0) {  // special case for the first range
        primes[0] = 2;  // 1 is not a prime, but 2 is.
    }

    // synchronize
    pthread_mutex_lock(&wo->sd->mtx);      // lock the mutex
    // only 1 thread at a time reaches here
    // check the predicate: That is's this thread's turn to print
    while(wo->val != wo->sd->whos_turn) {  // <- the predicate
        // if control enters here, it was not this thread's turn

        // cond_wait internally "unlocks" the mutex to let other threads
        // reach here and wait for the condition variable to get signalled
        pthread_cond_wait(&wo->sd->cv, &wo->sd->mtx);

        // and here the lock is only held by one thread at a time again
    }
    // only the thread whos turn it is reaches here

    // print the collected primes
    for(int i = 0; i < found_count; ++i)
        printf("%ju\n", primes[i]);

    // step the "whos_turn" indicator
    wo->sd->whos_turn++;

    pthread_mutex_unlock(&wo->sd->mtx);  // release the mutex
    pthread_cond_broadcast(&wo->sd->cv); // signal all threads to check the predicate
    pthread_exit(NULL);
}

#define Size(x) (sizeof (x) / sizeof *(x))

int main() {
    shared_data sd = {.whos_turn = 0,
                      .mtx = PTHREAD_MUTEX_INITIALIZER,
                      .cv = PTHREAD_COND_INITIALIZER};
    pthread_t p[100];

    work_order wos[Size(p)];

    for (unsigned i = 0; i < Size(p); i++) {
        wos[i].val = i;  // the thread-unique information
        wos[i].sd = &sd; // all threads will point at the same `shared_data`
        pthread_create(&p[i], NULL, findPrimes, &wos[i]);
    }
    for (unsigned i = 0; i < Size(p); i++) {
        pthread_join(p[i], NULL);
    }
}