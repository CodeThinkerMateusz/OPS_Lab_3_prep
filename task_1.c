#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

struct thread_data {
    int thread_id;
    int num_tries;
    int local_hits;
};

void* monte_carlo_worker(void * arg){
    struct thread_data * data = (struct thread_data *)arg;
    int  id = data->thread_id;
    int  num_tries = data->num_tries;
    unsigned int seed = time(NULL) + id;
    int  local_hits = 0;

    for(int i = 0; i < num_tries ; i++){
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;

        double d = x*x + y*y;
        if(d <= 1 ){
            local_hits++;
        }
    }
    data->local_hits = local_hits;
    return NULL;

}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        return 1;
    }
    int k = atoi(argv[1]);
    // number of threads 
    int n = atoi(argv[2]);  
    //number of tries per thread

    pthread_t ids[k];   // this  is  system id 
    
    struct thread_data *threads[k]; 

    
    for(int i = 0; i < k; i++) {
        threads[i] = malloc(sizeof(struct thread_data));
        if(threads[i] == NULL)ERR("malloc");
        threads[i]->num_tries = n;
        threads[i]->local_hits = 0;
        threads[i]->thread_id = i;  // this  is  name  i am giving  to this  thread 
        if(pthread_create(&ids[i], NULL, monte_carlo_worker , (void *)threads[i]) != 0 )
            ERR("Couldn't create  thread ");

    }


    long sum = 0;

    for(int i = 0; i < k ; i++){
        pthread_join(ids[i], NULL);
        sum += threads[i]->local_hits;
        free(threads[i]);
    }
    double pi_aprox = (4.0 * sum) / ((long long)k * n);

    printf("The pi aprox is : %lf\n", pi_aprox);

    return 0;
}