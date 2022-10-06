#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define NUM_INVITADOS 10

struct semaforos{
    sem_t sem_manucho_puede_sentarse;
    sem_t sem_servir_comida;
    sem_t sem_pregunta;
    sem_t sem_respuesta;
    sem_t sem_levantarse;
};

struct parametro{
    struct semaforos semaforos_param;
    int platos_servidos;
    int invitados_sentados;
};

void* ejecutarHiloManucho(void *data) {

    struct parametro *mydata = data;

    printf("Inicia hilo Manucho\n");

    sem_wait(&(mydata->semaforos_param.sem_manucho_puede_sentarse));

    printf("Manucho Sentado\n");

    sem_post(&mydata->semaforos_param.sem_servir_comida);


    pthread_exit(0);
}

void* ejecutarHiloSentarInvitados(void *data) {

    struct parametro *mydata = data;

    printf("Invitados sentados: %d \n " , mydata->invitados_sentados);

    mydata->invitados_sentados += 1;

    if(mydata->invitados_sentados == NUM_INVITADOS) {
        printf("Todos sentados \n ");
        sem_post(&mydata->semaforos_param.sem_manucho_puede_sentarse);
    }

    pthread_exit(0);
}

void* ejecutarHiloInvitados(void *data) {

    struct parametro *mydata = data;
	mydata->platos_servidos = 0;
	mydata->invitados_sentados = 0;

    printf("Inicia hilo Invitados\n");
    pthread_t thread[NUM_INVITADOS];

    for(int i = 0; i < NUM_INVITADOS; i++) {
        pthread_create( &(thread[i]) , NULL, ejecutarHiloSentarInvitados , mydata);
    }
    for(int i = 0; i < NUM_INVITADOS; i++) {
        pthread_join(thread[i] , NULL);
    }

    pthread_exit(0);
}

void* ejecutarHiloMozos(void *data) {

    struct parametro *mydata = data;
    
    printf("Inicia hilo Mozos\n");

    sem_wait(&(mydata->semaforos_param.sem_servir_comida));

    printf("servir comida !\n");

    pthread_exit(0);
}

int main () {

	int rc;

    pthread_t th_manucho; 
    pthread_t th_invitados; 
    pthread_t th_mozos; 

    struct parametro *pthread_data = malloc(sizeof(struct parametro));
	
    sem_t sem_manucho_puede_sentarse;
    sem_t sem_servir_comida;
    sem_t sem_pregunta;
    sem_t sem_respuesta;
    sem_t sem_levantarse;

    pthread_data->semaforos_param.sem_manucho_puede_sentarse = sem_manucho_puede_sentarse;
    pthread_data->semaforos_param.sem_servir_comida = sem_servir_comida;
    pthread_data->semaforos_param.sem_pregunta = sem_pregunta;
    pthread_data->semaforos_param.sem_respuesta = sem_respuesta;
    pthread_data->semaforos_param.sem_levantarse = sem_levantarse;

	sem_init(&(pthread_data->semaforos_param.sem_manucho_puede_sentarse),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_servir_comida),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_pregunta),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_respuesta),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_levantarse),0,0);

    rc = pthread_create(
        &th_manucho,                           //identificador unico
        NULL,                          //atributos del thread
        ejecutarHiloManucho,             //funcion a ejecutar
        pthread_data
    ); 

    rc = pthread_create(
        &th_invitados,                           //identificador unico
        NULL,                          //atributos del thread
        ejecutarHiloInvitados,             //funcion a ejecutar
        pthread_data
    ); 

    rc = pthread_create(
        &th_mozos,                           //identificador unico
        NULL,                          //atributos del thread
        ejecutarHiloMozos,             //funcion a ejecutar
        pthread_data
    ); 

    pthread_join (th_manucho,NULL);
	pthread_join (th_invitados,NULL);
	pthread_join (th_mozos,NULL);

	sem_destroy(&sem_manucho_puede_sentarse);
	sem_destroy(&sem_servir_comida);
	sem_destroy(&sem_pregunta);
	sem_destroy(&sem_respuesta);
	sem_destroy(&sem_levantarse);

    pthread_exit(NULL);
}

