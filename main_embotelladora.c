#include "embotelladora.h"
#include <pthread.h>
#include <stdio.h>

int main() {
    // Inicializar los trenes
    inicializarTren(&tren_lavado30);
    inicializarTren(&tren_lavado50);
    inicializarTren(&tren_calidad);
    inicializarTren(&tren_llenado30);
    inicializarTren(&tren_llenado50);


    //asignacion de ids
    tren_lavado30.id = 1;
    tren_lavado50.id = 1;
    tren_calidad.id = 2;
    tren_llenado30.id = 3;
    tren_llenado50.id = 3;



    pthread_t th_prod30, th_prod50;
    pthread_t th_lc30, th_lc50;
    pthread_t th_cl1, th_cl2;
    pthread_t th_cons30, th_cons50;
    pthread_t th_listar;

    // Crear hilos de producción
    pthread_create(&th_prod30, NULL, Th_prod30, NULL);
    pthread_create(&th_prod50, NULL, Th_prod50, NULL);

    // Crear hilos de lavado
    pthread_create(&th_lc30, NULL, Th_lc30, NULL);
    pthread_create(&th_lc50, NULL, Th_lc50, NULL);

    // Crear hilos de control de calidad
    pthread_create(&th_cl1, NULL, Th_cl, NULL);
    pthread_create(&th_cl2, NULL, Th_cl, NULL);

    // Crear hilos de llenado
    pthread_create(&th_cons30, NULL, Th_cons30, NULL);
    pthread_create(&th_cons50, NULL, Th_cons50, NULL);

    // Crear hilo que lista el estado cada 5 segundos
    pthread_create(&th_listar, NULL, listar, NULL);

    // Esperar a que terminen (en este ejemplo, los hilos corren indefinidamente)
    pthread_join(th_prod30, NULL);
    pthread_join(th_prod50, NULL);
    pthread_join(th_lc30, NULL);
    pthread_join(th_lc50, NULL);
    pthread_join(th_cl1, NULL);
    pthread_join(th_cl2, NULL);
    pthread_join(th_cons30, NULL);
    pthread_join(th_cons50, NULL);
    pthread_join(th_listar, NULL);

    return 0;
}
