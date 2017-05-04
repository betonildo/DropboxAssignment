#include	<pthread.h>
#include	<semaphore.h>
#include	<stdlib.h>
#include	<stdio.h>

#define	N 2
#define	TRUE 1

int	buffer[N], in = 0, out = 0;
sem_t	vazio, cheio, mutexC, mutexP;

void *produtor(void *arg) {

   while(TRUE) {
	  sleep(rand()%5); 

      sem_wait(&vazio);
      sem_wait(&mutexP);
      buffer[in] = rand() % 100;
      printf("Produzindo buffer[%d] = %d\n", in, buffer[in]);
      in= (in+1) % N;
      sem_post(&mutexP);
      sem_post(&cheio);	   
   }
}

void *consumidor(void *arg) {

   while(TRUE) {
	  sleep(rand()%5); 

      sem_wait(&cheio);
      sem_wait(&mutexC);
      printf("Consumindo buffer[%d] = %d\n", out, buffer[out]);
      out = (out+1) % N;
      sem_post(&mutexC);
      sem_post(&vazio);	   
   }
}

int main(int argc, char *argv[ ]) {
    pthread_t cons, prod;

	sem_init(&mutexC, 0, 1);
    sem_init(&mutexP, 0, 1);
    sem_init(&vazio, 0, N);
    sem_init(&cheio, 0, 0);

    pthread_create(&prod, NULL, produtor, NULL);
    pthread_create(&cons, NULL, consumidor, NULL);
    
	pthread_exit(0);
}