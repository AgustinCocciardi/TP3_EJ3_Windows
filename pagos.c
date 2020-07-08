#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>

#define DNI 10
#define FECHA 12
#define DIA 10
#define REGISTROS 200
#define VALOR 30

typedef struct datos
{
    char pagoDni[REGISTROS][DNI];
    char pagoFecha[REGISTROS][FECHA];
    char asistenciaDni[REGISTROS][DNI];
    char asistenciaDia[REGISTROS][DNI];
} registros;

int main(int argc, char* argv[]){
    char *ayuda="-Help";
    if (argc == 2 && strcmp(argv[1],ayuda) == 0) //Muestro la ayuda al usuario
    {
        printf("\nEste programa tiene la funcionalidad de leer un archivo de pagos. Lo que debe hacer este programa es");
        printf("\nleer del archivo de pagos y enviar los datos a un proceso llamado 'Principal' que los procesará");
        printf("\nPara poder ejecutar este programa, debe correr primero los procesos 'Principal' y 'Asistencia'");
        printf("\nEjemplo de ejecución: ./Pagos");
        printf("\n");
        exit(3);
    }

    if (argc != 1)
    {
        puts("Exceso de parámetros. Escriba './Pagos -Help' para tener ayuda");
        exit(4);
    }

    //Abro el archivo y valido que haya podido abrirlo
    char *pagos="pagos.txt";

    FILE *archivoPagos; 
    archivoPagos=fopen(pagos,"r"); 

    if (archivoPagos == NULL)
    {
        printf("\nEl archivo no existe o no tiene permisos de lectura\n");
        exit(2);
    }

    key_t Clave;                    //clave para recursos compartidos
    int Id_Memoria;                 //Identificador de Memoria

    Clave = ftok("/bin/ls",VALOR);          //Pido una clave para recursos compartidos y verifico que haya podido recibirla
    if (Clave == (key_t) -1)
	{
		printf("No consigo clave para memoria compartida\n");
		exit(0);
    }

    Id_Memoria = shmget (Clave, sizeof(registros *)*1000, IPC_CREAT | 0666); //Pido ID para memoria compartida
    if (Id_Memoria == -1)
	{
		printf("No consigo Id para memoria compartida\n");
		exit (0);
    }

    registros* Memoria = (registros *)shmat (Id_Memoria, (registros *)0, 0);
    if(Memoria == NULL){
        printf("No pude conseguir memoria compartida\n");
        exit(0);
    }
    
    //Declaro los nombres de mis semaforos
    sem_t *semutex;
    sem_t *sepagos;
    sem_t *seasistencia;

    //Le hago attach a los semaforos ya creados
    semutex = sem_open("/mutex",0);
    sepagos = sem_open("/pagos",0);
    seasistencia = sem_open("/asistencia",0);

    //Me va a servir para leer del archivo
    char delimitador[]=";\n";
    char palabra[30]; 

    char* dni;
    char* fecha;

    //Hago un P() del semaforo pagos. Como está inicializado en 1, voy a entrar
    sem_wait(sepagos);
    sem_wait(semutex); //Tomo el Mutex para escribir en memoria compartida

    printf("\nContenido del archivo de pagos:\n");
    int cantidad = 0;
    while (feof(archivoPagos) == 0)
    {
        fgets(palabra,30,archivoPagos);
        dni = strtok(palabra,delimitador);
        fecha = strtok(NULL,delimitador);
        printf("DNI: %s\tFecha: %s\n", dni, fecha);
        //Escribo en memoria
        strcpy(Memoria->pagoDni[cantidad],dni);
        strcpy(Memoria->pagoFecha[cantidad],fecha);
        cantidad++;
    }
    //Termine de leer el archivo
    //Usaré la cadena 00 como corte de control
    strcpy(Memoria->pagoDni[cantidad],"00");
    strcpy(Memoria->pagoFecha[cantidad],"00");
    //printf("\nTerminó de escribir en memoria compartida\n");

    //Termine de escribir en memoria compartida
    sem_post(semutex); //Hago un V() al mutex
    sem_post(seasistencia); //Hago un V() al semaforo de asistencia. Así podré darle el paso

    
    //Cierro los semaforos
    sem_close(semutex);
    sem_close(sepagos);
    sem_close(seasistencia);

    //Cierro el archivo
    fclose(archivoPagos);
    //printf("\nRecursos cerrados\n");
}

