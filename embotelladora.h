#ifndef EMBOTELLADORA_H
#define EMBOTELLADORA_H

#include <time.h>
#include <pthread.h>

#define MAX_TREN_CAPACIDAD 5

#define BOTELLA_30 30
#define BOTELLA_50 50

// Tiempos de operación (definidos en embotellar.c)
extern const int Tprod30;
extern const int Tprod50;
extern const int Tlavado;
extern const int Tcalidad;
extern const int Tllenado30;
extern const int Tllenado50;

// Estructura de la botella
typedef struct {
    int id;
    int tamano;  // BOTELLA_30 o BOTELLA_50
    struct timespec tiempo_entrada;
} Botella;

// Estructura del tren (cola circular)
typedef struct {
    Botella cola[MAX_TREN_CAPACIDAD];
    int cuenta;
    int inicio;
    int fin;
    pthread_mutex_t mutex;
    pthread_cond_t cond_lleno;  
    pthread_cond_t cond_vacio;
    int id;
} Tren;

// Declaración externa de los trenes globales
extern Tren tren_lavado30;
extern Tren tren_lavado50;
extern Tren tren_calidad;
extern Tren tren_llenado30;
extern Tren tren_llenado50;

// Declaraciones de funciones auxiliares
void inicializarTren(Tren *tren);
int insertarEnTren(Tren *tren, Botella botella);
int extraerListo(Tren *tren, double tiempo_requerido, Botella *botella);
double getTimeDiff(struct timespec start, struct timespec end);
void sleep_fractional(double seconds);

// Declaraciones de las funciones de los hilos
void* Th_prod30(void *arg);
void* Th_prod50(void *arg);
void* Th_lc30(void *arg);
void* Th_lc50(void *arg);
void* Th_cl(void *arg);
void* Th_cons30(void *arg);
void* Th_cons50(void *arg);
void* listar(void *arg);

#endif // EMBOTELLADORA_H



