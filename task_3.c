#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

struct ThreadData{
    int* array;
    int elements;
    bool quit;
    pthread_mutex_t mutex;
};

void remove_element(struct ThreadData *data, int index)
{
   int i;
   for(i = index; i < data->elements - 1; i++) data->array[i] = data->array[i + 1];
   data->elements--;
}

void* thread_work(void* arg){
    struct ThreadData* data = (struct ThreadData*)arg;
    if (data == NULL) ERR("thread arg is NULL"); 

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals,SIGINT);
    sigaddset(&signals, SIGQUIT);

    while(data->quit == false){
        int sig;
        sigwait(&signals, &sig);
        if(sig == SIGINT){
            pthread_mutex_lock(&data->mutex);
            if(data->elements > 0){
                static unsigned int seed = 0;
                if(seed == 0)seed = (unsigned int)time(NULL);
                int index = rand_r(&seed) % data->elements;
                remove_element(data,index);
            }
            pthread_mutex_unlock(&data->mutex);
            
            
        }
        else if(sig == SIGQUIT){
            pthread_mutex_lock(&data->mutex);
            data->quit = true;
            pthread_mutex_unlock(&data->mutex);
        }
    }
    return NULL;
}

void usage(){
    printf("k - max number of array");
    exit(EXIT_FAILURE);
}
int main(int argc, char* argv[]){
    if(argc != 2)usage();

    int k = atoi(argv[1]);
    if(k < 1)usage();

    struct ThreadData thread;
    pthread_t thread_id;

    if(pthread_mutex_init(&thread.mutex, NULL) != 0)
        ERR("Could't initialize mutex");
    
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals,SIGINT);
    sigaddset(&signals, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &signals, NULL);

    thread.elements = k;
    thread.array = malloc(k*sizeof(int));
    for (int i = 0; i < k; i++)
    {
        thread.array[i] = i+1;
    }
    thread.quit = false;

    if(pthread_create(&thread_id, NULL, thread_work, &thread) != 0)
        ERR("Couldn't create thread");
    
    while(1){
        sleep(1);

        pthread_mutex_lock(&thread.mutex);
        if(thread.quit == true){
            pthread_mutex_unlock(&thread.mutex);
            break;
        }
        

        printf("[");
        for (int i = 0; i < thread.elements; i++)
        {
            if(i > 0) printf(",");
            printf(" %d ", thread.array[i]);    
        }
        printf(" ]\n");
        fflush(stdout);
        pthread_mutex_unlock(&thread.mutex);
    }

    pthread_join(thread_id, NULL);
    free(thread.array);
    pthread_mutex_destroy(&thread.mutex);
    
    return 0;
}
