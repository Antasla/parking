#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct vehiculo{ 
    int tipo;
    int matricula;
    int tiempoEntrada;
    int tiempoSalida;
} vehiculo; 

typedef struct nPlaza{
    int plaza;
    int planta;
} nPlaza;

#define coche 1
#define camion 2
#define true 1
#define false 0

int **parking; //Matriz
vehiculo *espera;
long nplazas;
long nplantas;
long ncoches;
long ncamiones;
long huecos;
long huecosDobles;
pthread_mutex_t mutex;
pthread_cond_t noLleno, noVacio; //Creo que hay que controlar esto de alguna manera, pero no sé :c




void matrixFree(int **matrix, long n);
void initParking();
void initCola();
void addCola();
void gestionEntradas();
void *entradaCoche();
void *entradaCamion();
void comprobarHuecos();
void controlTiempos();
int esperaVacia();
vehiculo vehiculoEnEspera();
nPlaza huecoVacioCoche();
nPlaza huecoVacioCamion();
void imprimirCola();
void prueba(void);


int main(int argc, char *argv[]){

    parking = (int **) malloc(nplazas * sizeof(int *));
    espera = (vehiculo *) malloc((ncoches + ncamiones*2) * sizeof(vehiculo *));

    /* ----- Reservar memoria para la matriz ----- */
	for (int i = 0; i < nplazas; ++i)
        {parking[i] = malloc(2 * sizeof(int));}

    printf("Inicializando parking...\n");
    initParking();

    nplazas = strtol(argv[1], NULL, 10);
    nplantas = strtol(argv[2], NULL, 10);
    
    printf("Asignando espacios...\n");
    if (argc < 3){
        printf("Argumentos inválidos\n");
    } else if (argc == 3){
        // Camiones = 0 y Coches = 2*Plazas*Plantas
        ncoches = 2 * nplantas * nplazas;
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

    /* Generamos una cola de coches y camiones, alternándolos */
    printf("Generando cola... \n");
    initCola();
    imprimirCola();
    /* Ponemos en marcha la entrada y salida */
    printf("Abriendo el parking... \n");
    gestionEntradas();

    matrixFree(parking, nplazas);
}

void matrixFree(int **matrix, long n) {
	int i;
	for(i = 0; i < n; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

void initParking(){
    
    for(int i=0; i < nplazas; i++)
        for(int j = 0; j < nplantas; j++)
            parking[i][j]=-1;

}

void initCola(){
    int auxCoches=ncoches, auxCamiones=ncamiones, i;
    printf("\n");
    while( i < (ncoches+(ncamiones*2))){
        if(auxCoches>0){
            espera[i].matricula=i;
            espera[i].tipo=1;
            printf("%do  ", espera[i].matricula);
            
            auxCoches--;
            i++;
        }
        if(auxCamiones>0){
            espera[i].matricula=i;
            espera[i+1].matricula=i;
            espera[i].tipo=2;
            espera[i+1].tipo=2;
            printf("%dc  ", espera[i].matricula);
            i=i+2;
            auxCamiones--;   
        }   
    }
    printf("\n");
}

void gestionEntradas(){

    vehiculo aux;

    pthread_t hiloCoches[ncoches], hiloCamiones[ncamiones];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&noLleno, NULL); 
    pthread_cond_init(&noVacio, NULL); 

    printf("Creando hilos para vehículos...");

    int i = 0;

    while (!esperaVacia()){
        aux = vehiculoEnEspera(); 
        //printf("%d\n", aux.tipo);
        if(aux.tipo == coche){
            
            if (0 != pthread_create(&hiloCoches[i], NULL, entradaCoche, NULL)){
                perror("No se ha podido crear hilo de coche\n");
                exit(-1);
            }
            printf("Hilo coche %d\n", i);

            espera[aux.matricula].matricula = -1;

        }else if(aux.tipo == camion){
            // Creamos los hilos con los argumentos para la función
            if (0 != pthread_create(&hiloCamiones[i], NULL, entradaCamion, NULL)){
                perror("No se ha podido crear hilo de camión\n");
                exit(-1);
            }
            printf("Hilo camión %d\n", i);

            espera[aux.matricula].matricula = -1;
            espera[aux.matricula+1].matricula = -1; 

        }else{ 
            //printf("No hay vehículo en espera\n"); //No debería llegar nunca aquí
        }
        i++;
    }

    pthread_mutex_destroy(&mutex); //Destruimos todo
    pthread_cond_destroy(&noLleno);
    pthread_cond_destroy(&noVacio);
    exit(0);
}

void *entradaCoche(){
    
    nPlaza auxPlaza;

    printf("Coche entrando ...\n");

    sleep((rand() % 10) + 5);
    /* Cuando despertamos, entramos en ZONA CRÍTICA */
    pthread_mutex_lock(&mutex);

    vehiculo auxCoche = vehiculoEnEspera();

    while(huecoVacioCoche().planta != -1){ //Esperamos a un hueco libre
        auxPlaza = huecoVacioCoche();
        pthread_cond_wait(&noLleno, &mutex);
    }

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCoche.matricula;

    printf("Coche con matrícula %d aparcado en plaza %d de la planta %d. \n", auxCoche.matricula, auxPlaza.plaza, auxPlaza.planta);

    pthread_mutex_unlock(&mutex);
    /* Salimos de la ZONA CRÍTICA, dormimos random y salimos del parking */
    
    sleep((rand() % 100) + 5); 
    /* Volvemos a entrar en ZONA CRÍTICA */
    pthread_mutex_lock(&mutex); 
    
    for(int i=0; i < nplazas; i++){
        for(int j = 0; j < nplantas; j++){
            if(parking[i][j] == auxCoche.matricula)
                parking[i][j] = -1;
        }
    }

    comprobarHuecos();
    
    printf("Coche con matrícula %d saliendo de plaza %d de la planta %d. \n", auxCoche.matricula, auxPlaza.plaza, auxPlaza.planta);

    pthread_cond_signal(&noLleno);
    pthread_mutex_unlock(&mutex); 
    pthread_exit(0);
}

void *entradaCamion(){
    
    nPlaza auxPlaza;

    printf("Camion entrando ...");

    sleep((rand() % 10) + 5);
    /* Cuando despertamos, entramos en ZONA CRÍTICA */
    pthread_mutex_lock(&mutex);

    while(huecoVacioCoche().planta != -1){ //Esperamos a un hueco libre
        auxPlaza = huecoVacioCamion();
        pthread_cond_wait(&noLleno, &mutex);
    }
    vehiculo auxCamion = vehiculoEnEspera();

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCamion.matricula;
    parking[auxPlaza.planta][auxPlaza.plaza + 1] = auxCamion.matricula;

    printf("Camión con matrícula %d aparcado en plaza %d de la planta %d. \n", auxCamion.matricula, auxPlaza.plaza, auxPlaza.planta);


    pthread_mutex_unlock(&mutex);
    /* Salimos de la ZONA CRÍTICA */

    sleep((rand() % 100) + 5); 
    /* Volvemos a entrar en ZONA CRÍTICA */
    pthread_mutex_lock(&mutex); 
    
    for(int i=0; i < nplazas; i++){
        for(int j = 0; j < nplantas; j++){
            if((j+1 < nplazas) && (parking[i][j] == auxCamion.matricula)){
                parking[i][j] = -1;
                parking[i][j+1] = -1;
            }   
        }
    }

    comprobarHuecos();
    
    printf("Camión con matrícula %d saliendo de plaza %d de la planta %d. \n", auxCamion.matricula, auxPlaza.plaza, auxPlaza.planta);

    pthread_cond_signal(&noLleno);
    pthread_mutex_unlock(&mutex); 
    pthread_exit(0);
}

void comprobarHuecos(){

    for(int i=0; i < nplazas; i++)
        for(int j = 0; j < nplantas; j++)
            if(parking[i][j] == -1)
                huecos++;

    for(int i=0; i < nplantas; i++)
        for(int j = 0; j < nplazas; j++)
            if ((nplazas < j+1) && (parking[i][j] == -1 && parking[i][j+1] == -1)) 
                huecosDobles++;
}

void controlTiempos(){

}

int esperaVacia(){
    for(int i = 0; i < (ncoches+ncamiones); i++){
        if(espera[i].matricula != -1){
            return false;
        }
    }
    printf("DEBUG1");
    return true;
}

vehiculo vehiculoEnEspera(){
    vehiculo aux;
    aux.matricula = -1;
    aux.tipo = -1;

    if (esperaVacia()){
        return aux;
    }
    
    for(int i = 0; i < (ncoches+ncamiones); i++){
        if(espera[i].matricula != -1){
            return espera[i];
        }
    }
    printf("Vehículo %d en espera. \n", aux.matricula);
    return aux;
}

nPlaza huecoVacioCoche(){
    nPlaza aux;
    aux.plaza = -1;
    aux.planta = -1;
    for(int i=0; i < nplantas; i++)
        for(int j = 0; j < nplazas; j++)
            if (parking[i][j] == -1){
                aux.planta = i;
                aux.plaza = j;
                return aux;
            }          
    return aux;      
}

nPlaza huecoVacioCamion(){
    nPlaza aux;
    aux.plaza = -1;
    aux.planta = -1;
    for(int i=0; i < nplantas; i++)
        for(int j = 0; j < nplazas; j++)
            if ((j < nplazas-1) && (parking[i][j] == -1) && (parking[i][j+1] == -1)){
                aux.planta = i;
                aux.plaza = j;
                return aux;
            }          
    return aux;  
}

void imprimirCola(){
    printf("\n");
    for(int i = 0; i < (ncoches+ncamiones*2); i++){
        printf("%d  ", espera[i].matricula);
    }
    printf("\n");
}

void prueba(void){
    printf("DEBUG");
}
