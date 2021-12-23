//
// Created by ismael on 9/12/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct vehiculo{ //De momento no hace falta, pero por si le metemos algún atributo más
    int tipo;
    int matricula;
} vehiculo; 

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



void matrixFree(int **matrix, long n);
void initParking();
void initCola();
void gestionEntradas();
void entrada(int vehiculo, int matricula);
void salida(int vehiculo, int matricula);
void comprobarHuecos();
void controlTiempos();
int esperaVacia();
vehiculo vehiculoEnEspera();
void prueba(void);


int main(int argc, char *argv[]){
    srand(time(0));
    parking = (int **) malloc(nplazas * sizeof(int *));
    espera = (vehiculo *) malloc((ncoches + ncamiones) * sizeof(int *));

    /* ----- Reservar memoria para la matriz ----- */
	for (int i = 0; i < nplazas; ++i)
        {parking[i] = malloc(2 * sizeof(int));}

    initParking();

    nplazas = strtol(argv[1], NULL, 10);
    nplantas = strtol(argv[2], NULL, 10);

    pthread_mutex_t t1, t2;

    pthread_mutex_init(&t1, NULL);
    pthread_mutex_init(&t1, NULL);
    

    if (argc < 3){
        printf("Argumentos inválidos\n");
    } else if (argc == 3){
        // Camiones = 0 y Coches = 2*Plazas*Plantas
        ncoches = 2 * nplantas * nplazas;
        initCola();
        printf("Plazas: %ld\n", nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
        pthread_create(&t1, NULL, prueba, NULL);
    } else if (argc == 4){
        // 3º arg -> Coches
        ncoches = strtol(argv[3], NULL, 10);
        initCola();
        printf("Plazas: %ld\n", nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
    } else if (argc == 5){
        // 3º arg -> Coches | 4º arg -> Camiones
        ncoches = strtol(argv[3], NULL, 10);
        ncamiones = strtol(argv[4], NULL, 10);
        initCola();
        printf("Plazas: %ld\n", nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
        printf("Camiones: %ld\n", ncamiones);
    }
    
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
    int auxCoches, auxCamiones, i;
    while( i < (ncoches+ncamiones)){
        if(auxCoches>0){
            espera[i].matricula=i;
            auxCoches--;
            i++;
            if(auxCamiones>0){
                espera[i].matricula=i;
                espera[i+1].matricula=i;
                i++;
            }
        }else if(auxCoches == 0){
            if(auxCamiones>0){
                espera[i].matricula=i;
                espera[i+1].matricula=i;
                i = i+2;
            }
        }        
    }
}

void gestionEntradas(){

    vehiculo aux;

    while (!esperaVacia()){
        comprobarHuecos();
        aux = vehiculoEnEspera();
        if(aux.tipo == coche){
            if(huecos > 0){
                //Lanzar un hilo con la función de entrada
                espera[aux.matricula].matricula = -1;
            }
        }else if(aux.tipo == camion){
            if(huecosDobles > 0){
                //Lanzar un hilo con la función de entrada
                espera[aux.matricula].matricula = -1;
                espera[aux.matricula+1].matricula = -1;
            }  
        }else{ //Ha pasado algo raro

        }
    }
}

void entrada(int vehiculo, int matricula){
    sleep(rand() % 100);
    comprobarHuecos();
    if(vehiculo == 1){ //Coche
        if(huecos < 1){
            //Habrá que esperar, supongo
        }else{
            /* ZONA CRÍTICA */
            for(int i=0; i < nplazas; i++){
                for(int j = 0; j < nplantas; j++){
                    if(parking[i][j] == -1){
                        parking[i][j] = matricula;
                        printf("Coche %d entrando...", matricula);
                    }
                }
            }
        }
    }else if(vehiculo == 2){ //Camión
        if(huecosDobles < 1){
            //Esperamos
        }else{
            /* ZONA CRÍTICA */
            for(int i=0; i < nplazas; i++){
                for(int j = 0; j < nplantas; j++){
                    if ((nplazas < j+1) && (parking[i][j] == -1 && parking[i][j+1] == -1)){ //TODO se va de memoria
                        parking[i][j] = matricula;
                        printf("Camión %d entrando...", matricula);
                    }
                }
            }
        } 
    }else{
        printf("Se ha producido un error");
    }
}

void salida(int vehiculo, int matricula){
    //TODO ¿vamos a implementar matriculas de camion y de coche distintas?
    for(int i=0; i < nplazas; i++){
        for(int j = 0; j < nplantas; j++){
            if(parking[i][j] == matricula)
                parking[i][j] = -1;
        }
    }

    comprobarHuecos();

    if(vehiculo == 1){
        printf("Coche %d saliendo...", matricula);
    }else if(vehiculo == 2){
        printf("Camión %d saliendo...", matricula);
    }else
        printf("Se ha producido un error");

    printf("Hay %ld huecos para coches:", huecos);
    printf("Hay %ld huecos para camiones:", huecosDobles);
}

void comprobarHuecos(){

    for(int i=0; i < nplazas; i++)
        for(int j = 0; j < nplantas; j++)
            if(parking[i][j] == -1)
                huecos++;

    for(int i=0; i < nplantas; i++)
        for(int j = 0; j < nplazas; j++)
            if ((nplazas < j+1) && (parking[i][j] == -1 && parking[i][j+1] == -1)) //TODO te vas de memoria
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
    return true;
}

vehiculo vehiculoEnEspera(){
    for(int i = 0; i < (ncoches+ncamiones); i++){
        if(espera[i].matricula != -1){
            return espera[i];
        }
    }
}


void prueba(void){
    printf("DEBUG");
}
