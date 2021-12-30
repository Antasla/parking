#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define VACIO -1

int **parking; // Matriz que nos servirá para aparcar los coches y camiones
long nplazas;
long nplantas;
long ncoches;
long ncamiones;
long plazas_ocupadas;
pthread_mutex_t mutex;
pthread_cond_t nuevoEspacio;


void matrixFree(int **matrix, long n);
void matrixFill(int **matrix);
void *addCamion(void *matricula);
void *addCoche(void *matricula);
void matrixShow(int **matrix);


int main(int argc, char *argv[]){
    
    srand(time(0));
    // 1º arg -> Plazas | 2º arg -> Plantas
    nplazas = strtol(argv[1], NULL, 10);
    nplantas = strtol(argv[2], NULL, 10);

    if (argc < 3){
        printf("Argumentos inválidos\n");
        return -1;
    } else if (argc == 3){
        // Camiones = 0 y Coches = 2*Plazas*Plantas   
        ncoches = 2 * nplantas * nplazas;
        ncamiones = 0;
        printf("Plazas: %ld\n", nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
    } else if (argc == 4){
        // 3º arg -> Coches
        ncoches = strtol(argv[3], NULL, 10);
        printf("Plazas: %ld\n", nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
    } else if (argc == 5){
        // 3º arg -> Coches | 4º arg -> Camiones
        ncoches = strtol(argv[3], NULL, 10);
        ncamiones = strtol(argv[4], NULL, 10);
        printf("Plazas: %ld\n", nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
        printf("Camiones: %ld\n", ncamiones);
    }
    /* ----- Reservar memoria para la matriz ----- */

    /* Reservar filas para el parking con capacidad para nplazas */
    parking = (int **) malloc(nplantas * sizeof(int *));

    /* Dentro de cada fila reservar las plazas * plantas que necesitemos */
    for (int i = 0; i < nplazas; ++i){
        parking[i] = (int *) malloc(nplazas * sizeof(int));
    }

    /* Para tener el control de cuantas plazas nos quedan en el parking */
    plazas_ocupadas = 0;

    /* Seteamos la matriz a VACIO */
    matrixFill(parking);
    matrixShow(parking);
    /* Creamos hilos para cada coche y camión  */
    pthread_t hilosCoches[ncoches];
    pthread_t hilosCamiones[ncamiones];

    /* Creamos los coches y los camiones */
    int coches[ncoches];
    int camiones[ncamiones];

    /* Para hacerlo más fácil sin struct, si la matrícula es mayor que 1000, es camión, si es menor, coche */
    for (int i = 0; i < ncamiones; ++i) {
        camiones[i] = 1000 + 1 + i;
    }

    for (int i = 0; i < ncoches; ++i) {
        coches[i] = i + 1;
    }

    /* Creamos los hilos con los argumentos para la función */
    for (int i = 0; i < ncamiones; ++i) {
        if (0 != pthread_create(&hilosCamiones[i], NULL, addCamion, &camiones[i])){
            perror("No se ha podido crear hilo de camión\n");
            return -1;
        }
    }

    for (int i = 0; i < ncoches; ++i) {
        if (0 != pthread_create(&hilosCoches[i], NULL, addCoche, &coches[i])){
            perror("No se ha podido crear hilo de coches\n");
            return -1;
        }
    }

    /* Inicializamos el mutex y la condición que usaremos para esperar cuando el parking esté lleno */
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&nuevoEspacio, NULL);

    while(TRUE);
}

void matrixFree(int **matrix, long n) {
	int i;
	for(i = 0; i < n; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

void matrixFill(int **matrix){
    for (int i = 0; i < nplantas; ++i) {
        for (int j = 0; j < nplazas; ++j) {
            matrix[i][j] = VACIO;
        }
    }
}

void matrixShow(int **matrix){
    for (int i = 0; i < nplantas; ++i) {
        for (int j = 0; j < nplazas; ++j) {
            printf("[%d] ",matrix[i][j]);
        }
        printf("\n");
    }
}

void *addCamion(void *matricula) {

    //TODO Manejar el tema de tener 2 huecos en vez de uno, la condición debería ser distinta

    int m = *((int *) matricula);
    printf("Matricula -> %d\n",m);
    int plaza_recien_ocupada[2];
    int planta_recien_ocupada;
    while (TRUE) {
        sleep((rand() % 100) + 5);
        /* Cuando despertamos, entramos en ZONA CRÍTICA */
        pthread_mutex_lock(&mutex);
        //Necesitamos mínimo 2 plazas para meter un camión
        while ((plazas_ocupadas == (nplazas * nplantas)) || (plazas_ocupadas == (nplazas * nplantas)-1)) {
            pthread_cond_wait(&nuevoEspacio, &mutex); // Esperamos a tener un hueco
        }
        /* Tenemos hueco, seguimos */
        for (int i = 0; i < nplantas; ++i) {
            for (int j = 0; j < nplazas; ++j) {
                if ((j+1<nplazas) && (parking[i][j] == VACIO) && (parking[i][j+1] == VACIO)) { // Tenemos un hueco para el coche
                    parking[i][j] = m;
                    parking[i][j+1] = m;
                    plazas_ocupadas += 2;
                    plaza_recien_ocupada[0] = j; // Plaza 1 ocupada
                    plaza_recien_ocupada[1] = j+1; // Plaza 2 ocupada
                    planta_recien_ocupada = i; // La planta que se encuentra la plaza
                    j++;
                }
            }
        }
        printf("Entrada al parking. CAMION %d en plazas %d y %d en planta %d.\n", m, plaza_recien_ocupada[0],
            plaza_recien_ocupada[1], planta_recien_ocupada);
        printf("Plazas libres: %ld\n", (nplazas * nplantas) - plazas_ocupadas);
        pthread_mutex_unlock(&mutex);
        /* Salimos de la ZONA CRÍTICA, dormimos random y salimos del parking */
        sleep((rand() % 100) + 5);
        /* Volvemos a entrar en ZONA CRÍTICA */
        pthread_mutex_lock(&mutex);
        parking[planta_recien_ocupada][plaza_recien_ocupada[0]] = VACIO;
        parking[planta_recien_ocupada][plaza_recien_ocupada[1]] = VACIO; // Ponemos el hueco del parking vacío
        plazas_ocupadas -= 2;
        printf("Salida al parking. COCHE %d en plaza %d en planta %d.\n", m, plaza_recien_ocupada[0],
               plaza_recien_ocupada[1]);
        printf("Plazas libres: %ld\n", (ncoches * nplantas) - plazas_ocupadas);
        /* Desbloqueamos al menos un thread que está bloqueado por el parking lleno */
        // https://linux.die.net/man/3/pthread_cond_signal
        pthread_cond_signal(&nuevoEspacio);
        pthread_mutex_unlock(&mutex);
    }
}

void *addCoche(void *matricula){
    int m = *((int *) matricula);
    printf("Matricula -> %d\n",m);
    int plaza_recien_ocupada[2];
    int encontrado;
    while(TRUE){
        sleep((rand() % 10) + 5);
        /* Cuando despertamos, entramos en ZONA CRÍTICA */
        pthread_mutex_lock(&mutex);
        while (plazas_ocupadas == (nplazas * nplantas)){
            pthread_cond_wait(&nuevoEspacio, &mutex); // Esperamos a tener un hueco
        }
        /* Tenemos hueco, seguimos */
        encontrado = FALSE;
        for (int i = 0; i < nplantas; ++i) {
            for (int j = 0; j < nplazas; ++j) {
                if (parking[i][j] == VACIO){ // Tenemos un hueco para el coche
                    parking[i][j] = m;
                    plazas_ocupadas += 1;
                    plaza_recien_ocupada[0] = i; // La planta que se encuentra la plaza
                    plaza_recien_ocupada[1] = j; // Plaza ocupada
                    encontrado = TRUE;
                    break;
                }
                if(encontrado == TRUE)
                    break;
            }
        }
        printf("Entrada al parking. COCHE %d en plaza %d en planta %d.\n", m, plaza_recien_ocupada[1], plaza_recien_ocupada[0]);
        //printf("Plazas libres: %ld\n", (nplazas * nplantas) - plazas_ocupadas);
        pthread_mutex_unlock(&mutex);
        /* Salimos de la ZONA CRÍTICA, dormimos random y salimos del parking */
        sleep((rand() % 10) + 5);
        /* Volvemos a entrar en ZONA CRÍTICA */
        pthread_mutex_lock(&mutex);
        parking[plaza_recien_ocupada[0]][plaza_recien_ocupada[1]] = VACIO; // Ponemos el hueco del parking vacío
        plazas_ocupadas -= 1;
        printf("Salida del parking. COCHE %d en plaza %d en planta %d.\n", m, plaza_recien_ocupada[1], plaza_recien_ocupada[0]);
        //printf("Plazas libres: %ld\n", (ncoches * nplantas) - plazas_ocupadas);
        /* Desbloqueamos al menos un thread que está bloqueado por el parking lleno */
        // https://linux.die.net/man/3/pthread_cond_signal
        /* Imprimimos el estado del parking */
        matrixShow(parking);

        pthread_cond_signal(&nuevoEspacio);
        pthread_mutex_unlock(&mutex);
    }
}