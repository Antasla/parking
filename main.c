#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct vehiculo{ //De momento no hace falta, pero por si le metemos algún atributo más
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
void gestionEntradas();
void *entradaCoche();
void *entradaCamion();
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

    /* Generamos una cola de coches y camiones, alternándolos */
    initCola();

    /* Ponemos en marcha la entrada y salida */
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

    pthread_t hiloCoches[ncoches], hiloCamiones[ncamiones];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&noLleno, NULL); //Revisar
    pthread_cond_init(&noVacio, NULL); //Revisar

    while (!esperaVacia()){
        comprobarHuecos();
        aux = vehiculoEnEspera();
        if(aux.tipo == coche){
            if(huecos > 0){
                /* Creamos los hilos con los argumentos para la función */
                for (int i = 0; i < ncamiones; ++i) {
                    if (0 != pthread_create(&hiloCoches[i], NULL, entradaCoche, &espera)){
                        perror("No se ha podido crear hilo de coche\n");
                        exit(-1);
                    }
                }
                /* pthread_join(hiloCoches, NULL); */ //Esperamos a que termine
                espera[aux.matricula].matricula = -1;
            }
        }else if(aux.tipo == camion){
            if(huecosDobles > 0){
                /* Creamos los hilos con los argumentos para la función */
                for (int i = 0; i < ncamiones; ++i) {
                    if (0 != pthread_create(&hiloCamiones[i], NULL, entradaCamion, &espera)){
                        perror("No se ha podido crear hilo de camión\n");
                        exit(-1);
                    }
                }
                /* pthread_join(hiloCoches, NULL);
                pthread_join(hiloCoches, NULL); */
                espera[aux.matricula].matricula = -1;
                espera[aux.matricula+1].matricula = -1;
            }  
        }else{ 
            printf("No hay vehículo en espera"); //No debería llegar nunca aquí
        }
    }
    /* Inicializamos el mutex y la condición que usaremos para esperar cuando el parking esté lleno */
/*     pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&noLleno, NULL);
    pthread_cond_init(&noVacio, NULL);
 */

    pthread_mutex_destroy(&mutex); //Destruimos todo
    pthread_cond_destroy(&noLleno);
    pthread_cond_destroy(&noVacio);
    exit(0);
}

void *entradaCoche(){
    
    nPlaza auxPlaza;

    printf("Coche entrando ...");

    sleep((rand() % 10) + 5);
    /* Cuando despertamos, entramos en ZONA CRÍTICA */
    pthread_mutex_lock(&mutex);

    while(huecoVacioCoche().planta != -1) //Esperamos a un hueco libre, esto habría que controlarlo con la condición
        auxPlaza = huecoVacioCoche();

    vehiculo auxCoche = vehiculoEnEspera();

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCoche.matricula;

    printf("Coche con matrícula %d aparcado en plaza %d de la planta %d. \n", auxCoche.matricula, auxPlaza.plaza, auxPlaza.planta);

    pthread_mutex_unlock(&mutex);
    /* Salimos de la ZONA CRÍTICA, dormimos random y salimos del parking */
    
    //sleep((rand() % 100) + 5); //Esto es de la salida y pensaba hacerlo a parte
    /* Volvemos a entrar en ZONA CRÍTICA */
  /*   pthread_mutex_lock(&mutex); 
    
    for(int i=0; i < nplazas; i++){
        for(int j = 0; j < nplantas; j++){
            if(parking[i][j] == auxCoche.matricula)
                parking[i][j] = -1;
        }
    }

    comprobarHuecos();
    
    printf("Coche con matrícula %d saliendo de plaza %d de la planta %d. \n", auxCoche.matricula, auxPlaza.plaza, auxPlaza.planta);

    pthread_cond_signal(&noLleno);
    pthread_mutex_unlock(&mutex); */
}

void *entradaCamion(){
    
    nPlaza auxPlaza;

    printf("Camion entrando ...");

    sleep((rand() % 10) + 5);
    /* Cuando despertamos, entramos en ZONA CRÍTICA */
    pthread_mutex_lock(&mutex);

    while(huecoVacioCoche().planta != -1) //Esperamos a un hueco libre, esto habría que controlarlo con la condición
        auxPlaza = huecoVacioCamion();

    vehiculo auxCamion = vehiculoEnEspera();

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCamion.matricula;
    parking[auxPlaza.planta][auxPlaza.plaza + 1] = auxCamion.matricula;

    printf("Camión con matrícula %d aparcado en plaza %d de la planta %d. \n", auxCamion.matricula, auxPlaza.plaza, auxPlaza.planta);

    while(huecoVacioCoche().planta != -1) //Esperamos a un hueco libre, esto habría que controlarlo con la condición
        auxPlaza = huecoVacioCoche();

    vehiculo auxCoche = vehiculoEnEspera();

    parking[auxPlaza.planta][auxPlaza.plaza] = auxCoche.matricula;

    pthread_mutex_unlock(&mutex);
    /* Salimos de la ZONA CRÍTICA */
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
