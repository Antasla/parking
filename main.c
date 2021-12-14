//
// Created by ismael on 9/12/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

typedef struct vehiculo{ //De momento no hace falta, pero por si le metemos algún atributo más
    int tipo;
    int matricula;
} vehiculo; 

#define coche 1
#define camion 2

int **parking; //Matriz
vehiculo *espera;
long nplazas;
long nplantas;
long ncoches;
long ncamiones;
long huecos;
long huecosDobles;



void matrixFree(int **matrix, int n);
void entrada(int vehiculo, int matricula);
void salida(int vehiculo, int matricula);
void comprobarHuecos();
void controlTiempos();
void llenarEspera();


int main(int argc, char *argv[]){

    parking = (int **) malloc(nplazas * sizeof(int *));
    espera = (vehiculo *) malloc((ncoches+ncamiones) * sizeof(int *));

    /* ----- Reservar memoria para la matriz ----- */
	for (int i = 0; i < nplazas; ++i)
		parking[i] = malloc(2 * sizeof(int));

    if (argc < 3){
        printf("Argumentos inválidos\n");
    } else if (argc == 3){
        // 1º arg -> Plazas | 2º arg -> Plantas
        // Camiones = 0 y Coches = 2*Plazas*Plantas
        nplazas = strtol(argv[1], NULL, 10);
        nplantas = strtol(argv[2], NULL, 10);
        ncoches = 2*nplantas*nplazas;
        printf("Plazas: %ld\n",nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
    } else if (argc == 4){
        // 1º arg -> Plazas | 2º arg -> Plantas | 3º arg -> Coches
        nplazas = strtol(argv[1], NULL, 10);
        nplantas = strtol(argv[2], NULL, 10);
        ncoches = strtol(argv[3], NULL, 10);
        printf("Plazas: %ld\n",nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
    } else if (argc == 5){
        // 1º arg -> Plazas | 2º arg -> Plantas | 3º arg -> Coches | 4º arg -> Camiones
        nplazas = strtol(argv[1], NULL, 10);
        nplantas = strtol(argv[2], NULL, 10);
        ncoches = strtol(argv[3], NULL, 10);
        ncamiones = strtol(argv[4], NULL, 10);
        printf("Plazas: %ld\n",nplazas);
        printf("Plantas: %ld\n", nplantas);
        printf("Coches: %ld\n", ncoches);
        printf("Camiones: %ld\n", ncamiones);
    }
    
    matrixFree(parking, nplazas);
}

void matrixFree(int **matrix, int n) {
	int i;
	for(i = 0; i < n; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

void entrada(int vehiculo, int matricula){
    comprobarHuecos();
    if(vehiculo==1){ //Coche
        if(huecos<1){
            //Habrá que esperar, supongo
        }else{
            for(int i=0; i < nplazas; i++){
                for(int j = 0; j < nplantas; j++){
                    if(parking[i][j] == -1){
                        parking[i][j] = matricula;
                        printf("Coche %d entrando...", matricula);
                    }
                }
            }
        }
    }else if(vehiculo==2){ //Camión
        if(huecosDobles<1){
            //Esperamos
        }else{
            for(int i=0; i < nplazas; i++){
                for(int j = 0; j < nplantas; j++){
                    if(parking[i][j] == -1 && parking[i][j+1] == -1){
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

    for(int i=0; i < nplazas; i++){
        for(int j = 0; j < nplantas; j++){
            if(parking[i][j] == matricula)
                parking[i][j] = -1;
        }
    }

    comprobarHuecos();

    if(vehiculo==1){
        printf("Coche %d saliendo...", matricula);
    }else if(vehiculo==2){
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
                huecos ++;

    for(int i=0; i < nplantas; i++)
        for(int j = 0; j < nplazas; j++)
            if(parking[i][j] == -1 && parking[i][j+1] == -1)
                huecosDobles ++;
}

void controlTiempos(){

}

void llenarEspera(){
    for (int i = 0; i<(ncoches+ncamiones); i++)
        espera[i].matricula = i;
}