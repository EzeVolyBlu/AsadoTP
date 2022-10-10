#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>
#include <time.h>

#define NUM_INVITADOS 10
#define NUM_COMENSALES (NUM_INVITADOS+1)
#define NUM_MOZOS 3

struct semaforos{
    sem_t sem_manucho_puede_sentarse;
    sem_t sem_servir_comida;
    sem_t sem_pregunta_mundialista;
    sem_t sem_respuesta;
    sem_t sem_levantarse;
    sem_t sem_invitados_sentados;
    sem_t sem_platos_servidos;
};

struct parametro{
    sem_t sems_empezar_a_comer[NUM_COMENSALES];
    int nro_invitado;
    struct semaforos semaforos_param;
};

void* lanzar_pregunta_mundialista(void *data) {

    struct parametro *mydata = data;
    printf("Manucho: ¿Quién trajo figuritas para cambiar? \n ");
    usleep(2 * 1000000);
    sem_post(&mydata->semaforos_param.sem_pregunta_mundialista);
    sem_wait(&mydata->semaforos_param.sem_respuesta);
    usleep(2 * 1000000);
    printf("**Manucho piensa... \n");
    int respuesta_aceptable = manuchoPiensaRespuesta();
    printf("respuesta_aceptable: %ld\n", respuesta_aceptable);

    if (respuesta_aceptable) {
        usleep(20 * 1000000);
    }
    sem_post(&mydata->semaforos_param.sem_levantarse);
    printf("**Manucho se levanta \n");

}

int manuchoPiensaRespuesta() {
    time_t seconds = time(NULL);
    int like = (seconds % 2 == 0);
    printf("like: %ld\n", like);

    if(like == 0) {
        printf("**Manucho se enoja D=< \n");
    } else {
        printf("**Manucho está feli :) \n");
    }
    return(like);
}

void* ejecutarHiloManucho(void *data) {

    struct parametro *mydata = data;

    printf("Inicia hilo Manucho\n");

    sem_wait(&(mydata->semaforos_param.sem_manucho_puede_sentarse));

    printf("Manucho Sentado\n");

    // habilitar que sirvan comida
    sem_post(&mydata->semaforos_param.sem_servir_comida);

    // esperando que le sirvan comida (manucho siempre está primero, queda para mejorar...)
    sem_wait(&(mydata->sems_empezar_a_comer[0]));

    printf("Manucho comiendo \n ");

    usleep(20 * 1000000);

    printf("**Manucho termina de comer \n ");

    lanzar_pregunta_mundialista(mydata);

    pthread_exit(0);
}

void* ejecutarHiloSentarInvitados(void *data) {

    struct parametro *mydata = data;
    
    int invitados_sentados;

    sem_post(&mydata->semaforos_param.sem_invitados_sentados);
    sem_getvalue(&mydata->semaforos_param.sem_invitados_sentados, &invitados_sentados);

    printf("Invitados sentados: %d \n " , invitados_sentados);

    if(invitados_sentados == NUM_INVITADOS) {
        printf("Todos sentados \n ");
        sem_post(&mydata->semaforos_param.sem_manucho_puede_sentarse);
    }

    sem_wait(&(mydata->sems_empezar_a_comer[invitados_sentados]));
    printf("Invitado comiendo: %d \n " , invitados_sentados);

    usleep(15 * 1000000);

    printf("**Invitado termina de comer \n ");

    // esperando que hagan la pregunta
    sem_wait(&mydata->semaforos_param.sem_pregunta_mundialista);

    int rta;
    sem_getvalue(&mydata->semaforos_param.sem_respuesta, &rta);
    if(rta < 1) {
        sem_post(&mydata->semaforos_param.sem_respuesta);
        printf("la respuesta es... 110100100110011010011010 \n ");
    }

    sem_post(&mydata->semaforos_param.sem_pregunta_mundialista);

    // sem_post(&mydata->semaforos_param.sem_pregunta_mundialista);
    sem_post(&mydata->semaforos_param.sem_respuesta);

    sem_wait(&mydata->semaforos_param.sem_levantarse);
    printf("**invitado se levanta \n ");
    sem_post(&mydata->semaforos_param.sem_levantarse);

    pthread_exit(0);
}

void* ejecutarHiloInvitados(void *data) {

    printf("Inicia hilo Invitados\n");

    // params
    struct parametro *mydata = data;
    mydata->nro_invitado;

    pthread_t thread[NUM_INVITADOS];

    for(int i = 0; i < NUM_INVITADOS; i++) {
        
        mydata->nro_invitado = i;
        pthread_create( &(thread[i]) , NULL, ejecutarHiloSentarInvitados , mydata);
        
        sem_t sem_empezar_a_comer;
        mydata->sems_empezar_a_comer[i] = sem_empezar_a_comer;
    }

    for(int i = 0; i < NUM_INVITADOS; i++) {

    	sem_init(&(mydata->sems_empezar_a_comer[i]),0,0);
        
        pthread_join(thread[i] , NULL);
    }

    pthread_exit(0);
}

void* ejecutarHiloServirComida(void *data) {

    struct parametro *mydata = data;
    
    int platos_servidos;

    sem_getvalue(&mydata->semaforos_param.sem_platos_servidos, &platos_servidos);

    while (platos_servidos < NUM_COMENSALES)
    {
        printf("platos servidos: %d \n " , platos_servidos);
        sem_post(&mydata->semaforos_param.sem_platos_servidos);
        usleep(2 * 1000000);
        // servirComida();
        sem_post(&mydata->sems_empezar_a_comer[platos_servidos]);
        usleep(2 * 1000000);
        sem_getvalue(&mydata->semaforos_param.sem_platos_servidos, &platos_servidos);
    }
    
    printf("final servidos: %d \n " , platos_servidos);

}


void* ejecutarHiloMozos(void *data) {

    struct parametro *mydata = data;
    
    printf("Inicia hilo Mozos\n");

    sem_wait(&(mydata->semaforos_param.sem_servir_comida));

    printf("servir comida !\n");


    pthread_t thread[NUM_MOZOS];

    for(int i = 0; i < NUM_MOZOS; i++) {
        pthread_create( &(thread[i]) , NULL, ejecutarHiloServirComida , mydata);
    }

    for(int i = 0; i < NUM_MOZOS; i++) {
        pthread_join(thread[i] , NULL);
    }


    pthread_exit(0);
}

int main () {

	int rc;

    pthread_t th_manucho; 
    pthread_t th_invitados; 
    pthread_t th_mozos; 

    struct parametro *pthread_data = malloc(sizeof(struct parametro));
	
    sem_t sem_invitados_sentados;
    sem_t sem_manucho_puede_sentarse;
    sem_t sem_servir_comida;
    sem_t sem_pregunta_mundialista;
    sem_t sem_respuesta;
    sem_t sem_levantarse;
    sem_t sem_platos_servidos;

    pthread_data->semaforos_param.sem_invitados_sentados = sem_invitados_sentados;
    pthread_data->semaforos_param.sem_manucho_puede_sentarse = sem_manucho_puede_sentarse;
    pthread_data->semaforos_param.sem_servir_comida = sem_servir_comida;
    pthread_data->semaforos_param.sem_pregunta_mundialista = sem_pregunta_mundialista;
    pthread_data->semaforos_param.sem_respuesta = sem_respuesta;
    pthread_data->semaforos_param.sem_levantarse = sem_levantarse;
    pthread_data->semaforos_param.sem_platos_servidos = sem_platos_servidos;

	sem_init(&(pthread_data->semaforos_param.sem_invitados_sentados),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_manucho_puede_sentarse),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_servir_comida),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_platos_servidos),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_pregunta_mundialista),0,0);
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
	sem_destroy(&sem_pregunta_mundialista);
	sem_destroy(&sem_respuesta);
	sem_destroy(&sem_levantarse);

    pthread_exit(NULL);
}

