/*
 * Problema do barbeiro dorminhoco.
 */ 
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

//parametros do problema:
#define N_CLIENTES 20 //numero de clientes a ser gerado
#define N_CADEIRAS 2 //numero de cadeiras na barbearia
#define N_TEMPO 3 //quanto o barbeiro demora pra cortar

sem_t sem_cadeiras;
sem_t sem_cad_barbeiro;
sem_t sem_cabelo_cortado;
sem_t sem_cliente_cadeira;

void* f_barbeiro(void *v) {
	while(1) {
		sem_wait(&sem_cliente_cadeira); //chama um cliente para a cadeira
		sleep(N_TEMPO); //espera 3 segundos
		printf("Barbeiro cortou o cabelo de um cliente. ");
		sem_post(&sem_cabelo_cortado); //acabou de cortar o cabelo
	}
	return NULL;
}

void* f_cliente(void* v) {
	int id = *(int*) v;
	srand( (unsigned)time(NULL) );

	printf("Cliente %d chega. ", id);
	if (sem_trywait(&sem_cadeiras) == 0) { //tenta entrar na fila
		printf("Cliente %d entrou na barbearia.\n", id);
		sem_wait(&sem_cad_barbeiro); //vai para o semáforo da cadeira do barbeiro
		printf("Cliente %d sentou na cadeira do barbeiro.\n", id);
		sem_post(&sem_cliente_cadeira); 
		sem_post(&sem_cadeiras); //sai da cadeira
		sem_wait(&sem_cabelo_cortado); //espera o cabelo ser cortado
		sem_post(&sem_cad_barbeiro);
		printf("Cliente %d deixou a barbearia.\n", id);
	} else
		printf("Cliente %d nao entrou na barbearia.\n", id);
	return NULL;
}

int main() {
	
	pthread_t thr_clientes[N_CLIENTES], thr_barbeiro; //cria threads
	int i, j, id[N_CLIENTES];
	
	sem_init(&sem_cadeiras, 0, N_CADEIRAS); //inicializa todos os semaforos
	sem_init(&sem_cad_barbeiro, 0, 1);
	sem_init(&sem_cliente_cadeira, 0, 0);
	sem_init(&sem_cabelo_cortado, 0, 0);
		
	pthread_create(&thr_barbeiro, NULL, f_barbeiro, NULL); //cria a thread barbeiro
		
	i = 0;
	while(i < N_CLIENTES){
		sleep(1);
		for(j = rand() % 3 + 1; j > 0 && i < N_CLIENTES; j--){
			id[i] = i;
			pthread_create(&thr_clientes[i], NULL, f_cliente, (void*) &id[i]);
			i++;
		}
	}
  
	for (i = 0; i < N_CLIENTES; i++) 
		pthread_join(thr_clientes[i], NULL);

	return 0;
}

