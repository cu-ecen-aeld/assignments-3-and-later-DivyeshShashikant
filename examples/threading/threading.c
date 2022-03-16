/*@Author: Divvyesh Patel
 *@Brief: Threading assignment Part-1
 */



#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    int t1, t2, p1, p2;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    //sleep before acquiring mutex, for time specified in wait_to_obtain
    t1 = usleep(thread_func_args->wait_to_obtain);
    if(t1)
    {
    	ERROR_LOG("usleep1");
    }
    
    //acquire the mutex
    p1 = pthread_mutex_lock(thread_func_args->mutex);
    if(p1)
    {
    	ERROR_LOG("mutex_lock");
    }
    
    //release time of mutex specified in wait_to_obtain
    t2 = usleep(thread_func_args->wait_to_release);
    if(t2)
    {
    	ERROR_LOG("usleep2");
    }
    
    //release the mutex
    p2 = pthread_mutex_unlock(thread_func_args->mutex);
    if(p2)
    {
    	ERROR_LOG("mutex_unlock");
    }
    
    thread_func_args->thread_complete_success = true;
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     * 
     * See implementation details in threading.h file comment block
     */
     
     //create instance for the data structure
     struct thread_data *thread_data;
     
     //allocate memory for the data structure
     thread_data = malloc(sizeof(thread_data));
     
     //obtain the mutex
     thread_data->mutex = mutex;
     
     //wait time to obtain mutex
     thread_data->wait_to_obtain =  wait_to_obtain_ms * 1000;
     
     //wait time to release the mutex
     thread_data->wait_to_release = wait_to_release_ms *1000;
     
     int t1;
     
     //create thread
     t1 = pthread_create(thread,NULL,threadfunc,thread_data);
     if(t1)
     {
     	ERROR_LOG("pthread_create");
     	return false;
     }
     
     return true;
}

