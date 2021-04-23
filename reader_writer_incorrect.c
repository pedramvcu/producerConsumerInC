#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
/***************************************************************/
//N is the total readers here
#define N 10

/*
This program provides a possible solution for first readers writers problem using mutex and semaphore.
I have used 10 readers and 5 producers to demonstrate the solution. You can always play with these values.
*/
struct cs {
  int val;
  sem_t B1;
  sem_t B2;
};

void my_sem_init(struct cs *csVal, int k){
  csVal->val=k;
  sem_init(&csVal->B1, 0, 0);
  sem_init(&csVal->B2, 0, 1);

}

void my_sem_wait(struct cs *csVal){
  sem_wait(&csVal->B2);
  csVal->val=csVal->val-1;
  if(csVal->val<0){
    sem_post(&csVal->B2);
    sem_wait(&csVal->B1);
  }else{
    sem_post(&csVal->B2);
  }

}

void my_sem_post(struct cs *csVal){
  sem_wait(&csVal->B2);
  csVal->val=csVal->val+1;
  if(csVal->val<=0){
    sem_post(&csVal->B1);
  }
  sem_post(&csVal->B2);

}

sem_t wrt;
struct cs readSem;
pthread_mutex_t mutex;
int cnt = 1;
int numreader = 0;

void *writer(void *wno)
{   
    sem_wait(&wrt);
    cnt = cnt*2;
    printf("Writer %d modified cnt to %d\n",(*((int *)wno)),cnt);
    sem_post(&wrt);

}
void *reader(void *rno)
{   
    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutex);
    numreader++;
    if(numreader == 1) {
        sem_wait(&wrt); // If this id the first reader, then it will block the writer
    }
    
    pthread_mutex_unlock(&mutex);
	my_sem_wait(&readSem);
    // Reading Section
    printf("Reader %d: read cnt as %d\n",*((int *)rno),cnt);
    sleep(10);
    printf("Reader %d completed: read cnt as %d\n",*((int *)rno),cnt);
	my_sem_post(&readSem);
	

    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutex);
    numreader--;
    if(numreader == 0) {
        sem_post(&wrt); // If this is the last reader, it will wake up the writer.
    }
    pthread_mutex_unlock(&mutex);
    
}

int main()
{   

    pthread_t read[10],write[5];
    pthread_mutex_init(&mutex, NULL);
    sem_init(&wrt,0,1);
    //new sem added
    my_sem_init(&readSem,N);

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer

    for(int i = 0; i < 10; i++) {
        pthread_create(&read[i], NULL, (void *)reader, (void *)&a[i]);
    }
    for(int i = 0; i < 5; i++) {
        pthread_create(&write[i], NULL, (void *)writer, (void *)&a[i]);
    }

    for(int i = 0; i < 10; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&wrt);
	//free(&readSem);

    return 0;
    
}