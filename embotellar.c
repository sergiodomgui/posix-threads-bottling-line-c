#include "embotelladora.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Definición de los tiempos de operación (en segundos)
const int Tprod30    = 4;
const int Tprod50    = 5;
const int Tlavado    = 15;
const int Tcalidad   = 16;
const int Tllenado30 = 13;
const int Tllenado50 = 4;

//Otras variables
const int tamano_requerido = 150;

// Definición de los trenes globales
Tren tren_lavado30;
Tren tren_lavado50;
Tren tren_calidad;
Tren tren_llenado30;
Tren tren_llenado50;

// Variables globales para contar botellas en tránsito
int botellas_entre_trenes = 0;
pthread_mutex_t mutex_botellas_entre_trenes = PTHREAD_MUTEX_INITIALIZER;

// ======================================================
// Funciones auxiliares
// ======================================================



void inicializarTren(Tren *tren) {
    tren->cuenta = 0;
    tren->inicio = 0;
    tren->fin = 0;
    pthread_mutex_init(&tren->mutex, NULL);
    pthread_cond_init(&tren->cond_lleno, NULL);
    pthread_cond_init(&tren->cond_vacio, NULL);
}

// Función de insertar
int insertarEnTren(Tren *tren, Botella botella) {
    pthread_mutex_lock(&tren->mutex);
    if (tren->cuenta >= MAX_TREN_CAPACIDAD){
        if(tren->id == 1) printf("Esperando espacio en el tren de lavado para insertar botella %d.\n", botella.id);
        else if(tren->id == 2) printf("Esperando espacio en el tren de calidad para insertar botella %d.\n", botella.id);
        else if(tren->id == 3) printf("Esperando espacio en la estación de llenado para insertar botella %d.\n", botella.id);
    }
    while (tren->cuenta >= MAX_TREN_CAPACIDAD) {
        pthread_cond_wait(&tren->cond_lleno, &tren->mutex);
    }
    clock_gettime(CLOCK_MONOTONIC, &botella.tiempo_entrada);
    tren->cola[tren->fin] = botella;
    tren->fin = (tren->fin + 1) % MAX_TREN_CAPACIDAD;
    tren->cuenta++;
    pthread_cond_broadcast(&tren->cond_vacio);
    pthread_mutex_unlock(&tren->mutex);
    return 0;
}


// Función de extracción que espera hasta que la botella esté "lista" (tiempo mínimo cumplido)
// La botella permanece en el tren hasta que se complete el tiempo requerido.
int extraerListo(Tren *tren, double tiempo_requerido, Botella *botella) {

    while (1) {
        pthread_mutex_lock(&tren->mutex);
	
	if (tren->cuenta == 0){
		if(tren->id == 1) printf("Esperando por existencias en el tren de lavado.\n");
	     	else if(tren->id == 2) printf("Esperando por existencias en el tren de calidad.\n");
	    	else if(tren->id == 3) printf("Esperando por existencias en la estacion de llenado.\n");
	}
        while (tren->cuenta == 0) {

            pthread_cond_wait(&tren->cond_vacio, &tren->mutex);
        }
        // Observamos la botella en la cabeza del tren
        Botella b = tren->cola[tren->inicio];
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = getTimeDiff(b.tiempo_entrada, now);
        if (elapsed < tiempo_requerido) {
            double remaining = tiempo_requerido - elapsed;
            pthread_mutex_unlock(&tren->mutex);

            if(tren->id == 1) printf("Botella %d no lista en tren de lavado (requiere %.2f s, falta %.2f s)\n", b.id, tiempo_requerido, remaining);
            else if(tren->id == 2) printf("Botella %d no lista en tren de calidad (requiere %.2f s, falta %.2f s)\n", b.id, tiempo_requerido, remaining);
            else if(tren->id == 3) printf("Botella %d no lista en estacion de llenado (requiere %.2f s, falta %.2f s)\n", b.id, tiempo_requerido, remaining);


            sleep_fractional(remaining);
            continue;  // Vuelve a verificar
        }

        // La botella está lista; se extrae del tren
        *botella = tren->cola[tren->inicio];
        tren->inicio = (tren->inicio + 1) % MAX_TREN_CAPACIDAD;
        tren->cuenta--;
        pthread_cond_broadcast(&tren->cond_lleno);
        pthread_mutex_unlock(&tren->mutex);
        return 0;

    }
}



// Devuelve la diferencia de tiempo (en segundos) entre dos instantes
double getTimeDiff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// Función que permite dormir tiempos fraccionales usando nanosleep()
void sleep_fractional(double seconds) {
    if (seconds <= 0)
        return;
    struct timespec req;
    req.tv_sec = (time_t)seconds;
    req.tv_nsec = (long)((seconds - req.tv_sec) * 1e9);
    nanosleep(&req, NULL);
}

// ======================================================
// Funciones de los hilos
// ======================================================

// PRODUCCIÓN
// Produce botellas de 30cl (IDs pares)
void* Th_prod30(void *arg) {
    static int id_counter = 0;
    while (1) {
        Botella b;
        b.id = id_counter;
        id_counter += 2;
        b.tamano = BOTELLA_30;
        printf("Produciendo botella 30cl ID: %d\n", b.id);
        insertarEnTren(&tren_lavado30, b);
        sleep(Tprod30);
    }
    return NULL;
}

// Produce botellas de 50cl (IDs impares)
void* Th_prod50(void *arg) {
    static int id_counter = 1;
    while (1) {
        Botella b;
        b.id = id_counter;
        id_counter += 2;
        b.tamano = BOTELLA_50;
        printf("Produciendo botella 50cl ID: %d\n", b.id);
        insertarEnTren(&tren_lavado50, b);
        sleep(Tprod50);
    }
    return NULL;
}

// LAVADO
// En esta etapa se extrae la botella solo cuando lleva en el tren el tiempo mínimo (Tlavado)
void* Th_lc30(void *arg) {
    while (1) {
        Botella b;
        extraerListo(&tren_lavado30, Tlavado, &b);

        pthread_mutex_lock(&mutex_botellas_entre_trenes);
        botellas_entre_trenes++;
        pthread_mutex_unlock(&mutex_botellas_entre_trenes);

        insertarEnTren(&tren_calidad, b);
	
	pthread_mutex_lock(&mutex_botellas_entre_trenes);
        botellas_entre_trenes--;
        pthread_mutex_unlock(&mutex_botellas_entre_trenes);

        printf("Botella 30cl ID: %d completó lavado y fue trasladada a control de calidad\n", b.id);
    }
    return NULL;
}

void* Th_lc50(void *arg) {
    while (1) {
        Botella b;
        extraerListo(&tren_lavado50, Tlavado, &b);

	pthread_mutex_lock(&mutex_botellas_entre_trenes);
        botellas_entre_trenes++;
        pthread_mutex_unlock(&mutex_botellas_entre_trenes);

        insertarEnTren(&tren_calidad, b);
	
	pthread_mutex_lock(&mutex_botellas_entre_trenes);
        botellas_entre_trenes--;
        pthread_mutex_unlock(&mutex_botellas_entre_trenes);

        printf("Botella 50cl ID: %d completó lavado y fue trasladada a control de calidad\n", b.id);
    }
    return NULL;
}

// CONTROL DE CALIDAD

void* Th_cl(void *arg) {
    while (1) {
        Botella b;
        extraerListo(&tren_calidad, Tcalidad, &b);  
	
	pthread_mutex_lock(&mutex_botellas_entre_trenes);
        botellas_entre_trenes++;
        pthread_mutex_unlock(&mutex_botellas_entre_trenes);

        if(b.tamano == BOTELLA_30) {
            insertarEnTren(&tren_llenado30, b);
	    
	    pthread_mutex_lock(&mutex_botellas_entre_trenes);
	    botellas_entre_trenes--;
            pthread_mutex_unlock(&mutex_botellas_entre_trenes);

            printf("Botella 30cl ID: %d pasada a llenado \n", b.id);
        }
        else if(b.tamano == BOTELLA_50){
            insertarEnTren(&tren_llenado50, b);

	    pthread_mutex_lock(&mutex_botellas_entre_trenes);
       	    botellas_entre_trenes--;
            pthread_mutex_unlock(&mutex_botellas_entre_trenes);

            printf("Botella 50cl ID: %d pasada a llenado \n", b.id);
        }
    }
    return NULL;
}


// LLENADO
void* Th_cons30(void *arg) {
    while (1) {
        Botella b;
        extraerListo(&tren_llenado30, Tllenado30, &b);
        printf("Botella 30cl ID: %d completada y retirada de la línea\n", b.id);
    }
    return NULL;
}

void* Th_cons50(void *arg) {
    while (1) {
        Botella b;
        extraerListo(&tren_llenado50, Tllenado50, &b);
        printf("Botella 50cl ID: %d completada y retirada de la línea\n", b.id);
    }
    return NULL;
}

// LISTAR
// Muestra cada 5 segundos el estado de los trenes, incluyendo los IDs de las botellas en cada uno.
void* listar(void *arg) {
    while (1) {
        // Se bloquean los mutex de todos los trenes para un resultado consistente.
        pthread_mutex_lock(&tren_lavado30.mutex);
        pthread_mutex_lock(&tren_lavado50.mutex);
        pthread_mutex_lock(&tren_calidad.mutex);
        pthread_mutex_lock(&tren_llenado30.mutex);
        pthread_mutex_lock(&tren_llenado50.mutex);

        int total_botellas = tren_lavado30.cuenta + tren_lavado50.cuenta + 
                             tren_calidad.cuenta + tren_llenado30.cuenta + 
                             tren_llenado50.cuenta;

        printf("\n----- Estado de la Embotelladora -----\n");

        printf("[LAVADO 30cl] Botellas (%d): ", tren_lavado30.cuenta);
        for (int i = 0; i < tren_lavado30.cuenta; i++) {
            int pos = (tren_lavado30.inicio + i) % MAX_TREN_CAPACIDAD;
            printf("%d ", tren_lavado30.cola[pos].id);
        }
        printf("\n");

        printf("[LAVADO 50cl] Botellas (%d): ", tren_lavado50.cuenta);
        for (int i = 0; i < tren_lavado50.cuenta; i++) {
            int pos = (tren_lavado50.inicio + i) % MAX_TREN_CAPACIDAD;
            printf("%d ", tren_lavado50.cola[pos].id);
        }
        printf("\n");

        printf("[CONTROL DE CALIDAD] Botellas (%d): ", tren_calidad.cuenta);
        for (int i = 0; i < tren_calidad.cuenta; i++) {
            int pos = (tren_calidad.inicio + i) % MAX_TREN_CAPACIDAD;
            printf("%d ", tren_calidad.cola[pos].id);
        }
        printf("\n");

        printf("[LLENADO 30cl] Botellas (%d): ", tren_llenado30.cuenta);
        for (int i = 0; i < tren_llenado30.cuenta; i++) {
            int pos = (tren_llenado30.inicio + i) % MAX_TREN_CAPACIDAD;
            printf("%d ", tren_llenado30.cola[pos].id);
        }
        printf("\n");

        printf("[LLENADO 50cl] Botellas (%d): ", tren_llenado50.cuenta);
        for (int i = 0; i < tren_llenado50.cuenta; i++) {
            int pos = (tren_llenado50.inicio + i) % MAX_TREN_CAPACIDAD;
            printf("%d ", tren_llenado50.cola[pos].id);
        }
        printf("\n");

        // Desbloqueamos los mutex de los trenes
        pthread_mutex_unlock(&tren_lavado30.mutex);
        pthread_mutex_unlock(&tren_lavado50.mutex);
        pthread_mutex_unlock(&tren_calidad.mutex);
        pthread_mutex_unlock(&tren_llenado30.mutex);
        pthread_mutex_unlock(&tren_llenado50.mutex);

        // Se lee el contador de botellas en tránsito
        pthread_mutex_lock(&mutex_botellas_entre_trenes);
        int transito = botellas_entre_trenes;
        pthread_mutex_unlock(&mutex_botellas_entre_trenes);

        printf("\n[ENTRE TRENES] Botellas: %d\n", transito);

        total_botellas += transito;
        printf("\n[TOTAL] Botellas en el sistema: %d\n", total_botellas);
        printf("---------------------------------------\n\n");

        sleep(5);
    }
    return NULL;
}

