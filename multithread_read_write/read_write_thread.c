/*
 * read_write_thread.c
 *
 *  Created on: Aug 28, 2018
 *      Author: pournami
 */

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#define R_THREADS 5
#define W_THREADS 5
#define X 3

pthread_cond_t read_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t write_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int helper = 0;
int shrd_var = 0;
int readers = 0, writers = 0;

int *read_function(void *thread)
{
	int threadNum = *((int*)thread);
	for (int i = X; i > 0; i--) {
		sleep(0.1);
		pthread_mutex_lock(&mutex);
			while (helper < 0) {
				pthread_cond_wait(&read_cond, &mutex);
			}
			readers++;
			helper++;
		pthread_mutex_unlock(&mutex);

		/* read function */
		printf("value read = %d by thread = %d\n", shrd_var, threadNum);
		printf("No:of readers = %d\n", readers);

		pthread_mutex_lock(&mutex);
			readers--;
			helper--;
			if (helper == 0)
				pthread_cond_signal(&write_cond);
		pthread_mutex_unlock(&mutex);
	}
	return 0;

}

int *write_function(void *thread)
{
	int threadNum = *((int*)thread);
	int value = threadNum;
	for (int i = X; i > 0; i--) {
		sleep(0.1);
		pthread_mutex_lock(&mutex);
			while (helper != 0) {
				pthread_cond_wait(&write_cond, &mutex);
			}
			writers++;
			helper--;
		pthread_mutex_unlock(&mutex);

		/* write function */
		shrd_var = ++value;
		printf("value wrote = %d by thread = %d\n", shrd_var, threadNum);
		printf("No:of readers = %d\n", readers);

		pthread_mutex_lock(&mutex);
			writers--;
			helper++;
			if (readers > 0)
				pthread_cond_broadcast(&read_cond);
			else
				pthread_cond_signal(&write_cond);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}


int main(void)
{
	int r[R_THREADS], w[W_THREADS];
	pthread_attr_t attr;
	pthread_t read_threads[R_THREADS];
	pthread_t write_threads[W_THREADS];

	pthread_attr_init(&attr);
	for (int i = 0; i < R_THREADS; i++) {
		r[i] = i;
		w[i] = i;
		pthread_create(&read_threads[i], &attr, read_function, &r[i]);
		pthread_create(&write_threads[i], &attr, write_function, &w[i]);
	}

	for (int i = 0; i < R_THREADS; i++) {
		pthread_join(read_threads[i], NULL);
	}

	for (int i = 0; i < W_THREADS; i++) {
		pthread_join(write_threads[i], NULL);
	}
}
