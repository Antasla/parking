//
// Created by ismael on 9/12/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
    long nplazas, nplantas, ncoches, ncamiones;
    if (argc < 3){
        printf("Argumentos inválidos\n");
    } else if (argc == 3){
        // 1º arg -> Plazas | 2º arg -> Plantas
        // Camiones = 0 y Coches = 2*Plazas*Plantas
        nplazas = strtol(argv[1], NULL, 10);
        nplantas = strtol(argv[2], NULL, 10);
        printf("Plazas: %ld\n",nplazas);
        printf("Plantas: %ld\n", nplantas);
    } else if (argc == 4){
        // 1º arg -> Plazas | 2º arg -> Plantas | 3º arg -> Coches
        nplazas = strtol(argv[1], NULL, 10);
        nplantas = strtol(argv[2], NULL, 10);
        ncoches = strtol(argv[3], NULL, 10);
    } else if (argc == 5){
        // 1º arg -> Plazas | 2º arg -> Plantas | 3º arg -> Coches | 4º arg -> Camiones
        nplazas = strtol(argv[1], NULL, 10);
        nplantas = strtol(argv[2], NULL, 10);
        ncoches = strtol(argv[3], NULL, 10);
        ncamiones = strtol(argv[4], NULL, 10);
    }
}