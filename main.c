#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct vehiculo{ //De momento no hace falta, pero por si le metemos algún atributo más
    int tipo;
    int matricula;
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
void gestionEntradas();
void entradaCoche(void);
void entradaCamion(void);
void salida(int vehiculo, int matricula);
void comprobarHuecos();
void controlTiempos();
int esperaVacia();
vehiculo vehiculoEnEspera();
nPlaza huecoVacioCoche();
nPlaza huecoVacioCamion();
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

    initCola();
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

void gestionEntradas(){ //TENGO MUCHAS DUDAS CON ESTO

    vehiculo aux;
    pthread_t t1, t2;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&noLleno, NULL); //Revisar
    pthread_cond_init(&noVacio, NULL); //Revisar

    while (!esperaVacia()){
        comprobarHuecos();
        aux = vehiculoEnEspera();
        if(aux.tipo == coche){
            if(huecos > 0){
                pthread_create(&t1, NULL, entradaCoche, NULL);//Lanzar un hilo con la función de entrada 
                //No sé por qué da el fallo :c
                pthread_join(t1, NULL); //Esperamos a que termine
                espera[aux.matricula].matricula = -1;
            }
        }else if(aux.tipo == camion){
            if(huecosDobles > 0){
                pthread_create(&t1, NULL, entradaCamion, NULL);//Lanzar un hilo con la función de entrada
                pthread_join(t1, NULL);
                pthread_join(t1, NULL);
                espera[aux.matricula].matricula = -1;
                espera[aux.matricula+1].matricula = -1;
            }  
        }else{ 
            printf("No hay vehículo en espera"); //No debería llegar nunca aquí
        }
    }
    pthread_mutex_destroy(&mutex); //Destruimos todo
    pthread_cond_destroy(&noLleno);
    pthread_cond_destroy(&noVacio);
    exit(0);
}

void entradaCoche(void){
    
    printf("Coche entrando ...");
    
    nPlaza auxPlaza;

    if(huecoVacioCoche().planta != -1)
        auxPlaza = huecoVacioCoche();

    vehiculo auxCoche = vehiculoEnEspera();

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCoche.matricula;

    printf("Coche con matrícula %d aparcado en plaza %d de la planta %d. \n", auxCoche.matricula, auxPlaza.plaza, auxPlaza.planta);

    /* sleep(rand() % 100);
    comprobarHuecos();
    if(vehiculo == 1){ //Coche
        if(huecos < 1){
            //Habrá que esperar, supongo
        }else{
            // ZONA CRÍTICA 
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
            // ZONA CRÍTICA 
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
    } */
}

void entradaCamion(void){
    
    printf("Camion entrando ...");

    nPlaza auxPlaza = huecoVacioCamion();
    vehiculo auxCamion = vehiculoEnEspera();

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCamion.matricula;
    parking[auxPlaza.planta][auxPlaza.plaza + 1] = auxCamion.matricula;

    printf("Coche con matrícula %d aparcado en plaza %d de la planta %d. \n", auxCamion.matricula, auxPlaza.plaza, auxPlaza.planta);

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
    vehiculo aux;
    aux.matricula = -1;
    aux.tipo = -1;
    for(int i = 0; i < (ncoches+ncamiones); i++){
        if(espera[i].matricula != -1){
            return espera[i];
        }
    }
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
            if ((parking[i][j] == -1) && parking[i][j+1]){
                aux.planta = i;
                aux.plaza = j;
                return aux;
            }          
    return aux;  
}

void prueba(void){
    printf("DEBUG");
}
