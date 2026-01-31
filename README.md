# posix-threads-bottling-line-c

**SIMULACIÓN DE PLANTA EMBOTELLADORA CONCURRENTE**

**1. DESCRIPCIÓN DEL PROYECTO**
Esta aplicación simula el funcionamiento de una línea de producción de botellas 
en una planta embotelladora. El sistema gestiona botellas de dos 
capacidades (30cl y 50cl) a través de un flujo de trabajo concurrente 
implementado en C con la librería POSIX Threads (pthreads).

**2. ARQUITECTURA DEL SISTEMA**
El proceso se divide en etapas representadas por trenes (colas circulares):
- Lavado: Trenes independientes para 30cl y 50cl.
- Control de Calidad: Verificación de estado tras el lavado.
- Llenado: Etapa final diferenciada por tamaño (30cl y 50cl).

**3. TIEMPOS DE PROCESAMIENTO**
El sistema respeta los siguientes tiempos mínimos de operación:
- Producción: 4s (30cl) / 5s (50cl).
- Lavado: 15s (tiempo mínimo de permanencia).
- Control de Calidad: 16s (tiempo mínimo de inspección).
- Llenado: 13s (30cl) / 4s (50cl).

**4. MECANISMOS DE SINCRONIZACIÓN**
- Mutex: Cada tren posee su propio mutex para asegurar acceso exclusivo a la 
  cola y evitar condiciones de carrera.
- Variables de Condición: Se utilizan cond_lleno y cond_vacio para que los 
  hilos esperen espacio disponible o la llegada de nuevas botellas.
- Sincronización Temporal: La función extraerListo() garantiza que las botellas 
  no abandonen la etapa hasta cumplir el tiempo requerido mediante el uso 
  de nanosleep() y clock_gettime().
- Contador Global: Un mutex global protege el conteo de botellas que se 
  encuentran en tránsito entre las distintas estaciones.

**5. ESTRUCTURA DE ARCHIVOS**
- embotelladora.h: Definición de constantes, estructuras y prototipos.
- embotellar.c: Implementación de la lógica de hilos y gestión de colas.
- main_embotelladora.c: Inicialización del sistema y creación de hilos.

**6. COMPILACIÓN Y EJECUCIÓN**
Para compilar en sistemas Linux/Unix:
$ gcc -o embotelladora main_embotelladora.c embotellar.c -lpthread

Para ejecutar la simulación:
$ ./embotelladora
