#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h> 

void *estimar(void *);


//semaforo
sem_t block; 

//variables
double* X; 
double* Y;
double* Y_avgs;
double a;

typedef struct parametros{
	int init;
	int end;
	int it;
	int p;
}parametros;


int main(int argc, char* argv[]){
	// obtener parametros de la linea de comandos
	unsigned int seed = 1;
  	int p = 10000000;
  	int n_threads = 8;
  	int max_iters = 10;
	//
	int i,it;
	// variables para tiempo de ejecucion
	struct timeval t_start, t_end;
	double exec_time;

	// Getting input values
	int opt;
	while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
		switch(opt){  
			case 'p':  
			printf("vector size: %s\n", optarg);
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647);
			break;  
			case 's':  
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
			case 'n':  
			printf("threads number: %s\n", optarg);
			n_threads = strtol(optarg, NULL, 10);
			break;  
			case 'i':  
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;  
			case ':':  
			printf("option -%c needs a value\n", optopt);  
			break;  
			case '?':  
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}  
	}  
	srand(seed);

	printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", \
	 p, seed, n_threads, max_iters);	

	// initializing data
	X = (double*) malloc(sizeof(double) * p);
	Y = (double*) malloc(sizeof(double) * p);
	Y_avgs = (double*) malloc(sizeof(double) * max_iters);

	for(i = 0; i < p; i++){
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	for(i = 0; i < max_iters; i++){
		Y_avgs[i] = 0.0;
	}
	a = (double)rand() / RAND_MAX;

#ifdef DEBUG
	printf("vector X= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ",X[i]);
	}
	printf("%f ]\n",X[p-1]);

	printf("vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);

	printf("a= %f \n", a);	
#endif

	/*
	 *	Function to parallelize 
	 */
	gettimeofday(&t_start, NULL);
	
	//SAXPY iterative SAXPY mfunction
	
	int k = p/n_threads;
	sem_init(&block, 0, 1);
	pthread_t h[n_threads];
	parametros z[n_threads];
	it=0;
	
	while(it < max_iters){
		i=0;
		while(i <n_threads){
			z[i].init=i*k;
			if((i+1)!=n_threads){
				z[i].end=(i+1)*k;
			}else
				z[i].end=p+1;
			
			
			z[i].it=it;
			z[i].p=p;
			pthread_create(&h[i],NULL,estimar, &z[i]);
			i++;
		}
	       	for (i = 0; i <n_threads; i++)
			pthread_join(h[i], NULL);
		
		it ++;
	}
	gettimeofday(&t_end, NULL);

#ifdef DEBUG
	printf("RES: final vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);
#endif
	
	// Computing execution time
	exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
	exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
	printf("Execution time: %f ms \n", exec_time);
	printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
	printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);
	return 0;
}

void *estimar(void *args){
	 parametros *pa = (parametros *)args;
	 int i= pa->init;
	 int end = pa->end;
	 int it = pa->it;
	 int p = pa->p;
	 double t_yargs = 0.0;
	 do{
		Y[i] =Y[i] + a * X[i];
		t_yargs += Y[i];
		i++;
	}while(i<end);
	 sem_wait(&block);
	 Y_avgs[it] += t_yargs / p;
	 sem_post(&block);
	 pthread_exit(NULL);
}