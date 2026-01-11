#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include<unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define NUMBER_BINS 11

struct thread_data{
    pthread_mutex_t * mutex;
    int*  bins;
    int balls_to_throw;
    unsigned int seed;
    int  kubelek;
    int *threads_finished;
};

void* thread_work(void* arg){
    struct thread_data * data = (struct thread_data *)arg;
    for(int i = 0; i < data->balls_to_throw;i++){
        data->kubelek = 0;
        for(int j = 0; j <10;j++){
            int rd_num = rand_r(&data->seed) % 2;
            data->kubelek += rd_num;
        }
        pthread_mutex_lock(data->mutex);
        data->bins[data->kubelek]++;
        pthread_mutex_unlock(data->mutex);
    }
    pthread_mutex_lock(data->mutex);
    (*data->threads_finished)++;
    pthread_mutex_unlock(data->mutex);
    return NULL;
}

void  usage(){
    printf("k - number  of  threads");
    printf("n - number of  balls ");
}

int main(int argc, char* argv[]){
    if(argc != 3){
        usage();
    }
    int k = atoi(argv[1]);
    int n = atoi(argv[2]);
    int num_per_thread;
    int left = 0;
    if(n % k == 0) num_per_thread = n/k;
    else{
        left += n - (n/k)*k;
        num_per_thread = n/k;
    }

    int bins[NUMBER_BINS];
    struct thread_data data[k];
    pthread_t threads[k];

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,NULL);
    int threads_finished = 0;
    for(int i = 0; i  < NUMBER_BINS ;i++)bins[i] = 0;
    for(int i = 0 ; i < k ; i++){
        data[i].balls_to_throw = (i == k-1) ? num_per_thread + left : num_per_thread;
        data[i].bins = bins;
        data[i].seed = time(NULL) + i;
        data[i].mutex = &mutex;
        data[i].threads_finished = &threads_finished;
        if(pthread_create(&threads[i], NULL, thread_work, (void *)&data[i]) != 0 )
            ERR("Couldn't create  thread");
    }
    while(1){
        usleep(200);
        pthread_mutex_lock(&mutex);
        if(threads_finished == k){
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("Bins: \n");
    for(int i = 0 ; i < NUMBER_BINS ;i++){
        printf("Bin %d has %d balls \n", i, bins[i]);
    }

    double multipliers[NUMBER_BINS] = {300.0, 6.0, 2.0, 1.0, 0.3, 0.1, 0.3, 1.0, 2.0, 6.0, 300.0};

    double weighted_sum = 0.0;
    double total_weight = 0.0;

    for(int i = 0; i < NUMBER_BINS; i++){
        double weight = multipliers[i];
        weighted_sum += i * bins[i] * weight;
        total_weight += bins[i] * weight;
    }
    double mean = weighted_sum / total_weight;

    double total_payout = 0.0;

    for(int i = 0; i < NUMBER_BINS; i++){
        total_payout += bins[i] * multipliers[i];
    }      

    printf("Total balls thrown: %d\n", n);
    printf("Total payout: %.2f\n", total_payout);
    printf("Return multiplier: %.2fx\n", total_payout / n); 

    printf("Weighted mean: %.2f\n", mean);

}