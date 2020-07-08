#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
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
    char *ayuda="-Help"; //Muestro la ayuda
    if (argc == 2 && strcmp(argv[1],ayuda) == 0) //Muestro la ayuda al usuario
    {
        printf("\nEste programa tiene la funcionalidad de leer un archivo de asistencia. Lo que debe hacer este programa es");
        printf("\nleer del archivo de asistencia y enviar los datos a un proceso llamado 'Principal' que los procesará");
        printf("\nPara poder ejecutar este programa, debe correr primero el proceso 'Principal'");
        printf("\nEjemplo de ejecución: ./Asistencia");
        printf("\n");
        exit(3);
    }

    if (argc != 1)
    {
        puts("Exceso de parámetros. Escriba './Asistencia -Help' para tener ayuda");
        exit(4);
    }
    


    //Abro el archivo y valido que haya podido abrirlo
    char *asistencia="asistencia.txt";

    FILE *archivoAsistencia;
    archivoAsistencia=fopen(asistencia,"r");

    if(archivoAsistencia == NULL){
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
    sem_t *seasistencia;
    sem_t *sesocios;

    //Le hago attach a los semaforos ya creados
    semutex = sem_open("/mutex",0);
    seasistencia = sem_open("/asistencia",0);
    sesocios = sem_open("/socios",0);

    //Me va a servir para leer del archivo
    char delimitador[]=";\n";
    char palabra[20];

    char* dni;
    char* dia;

    //Hago un P() del semaforo pagos. Como está inicializado en 0, me quedo esperando a que el proceso Pagos haga un V() 
    sem_wait(seasistencia);
    sem_wait(semutex);  //Tomo el Mutex para escribir en memoria compartida

    printf("\nContenido del archivo de asistencia\n");
    int cantidad = 0;
    while (feof(archivoAsistencia) == 0)
    {
        fgets(palabra,20,archivoAsistencia);
        dni = strtok(palabra,delimitador);
        dia = strtok(NULL,delimitador);
        printf("DNI: %s\tDia: %s\n", dni, dia);
        //Escribo en memoria
        strcpy(Memoria->asistenciaDni[cantidad],dni);
        strcpy(Memoria->asistenciaDia[cantidad],dia);
        cantidad++;
    }
    //Termine de leer el archivo
    //Usaré la cadena 00 como corte de control
    //strcpy(Memoria->pagoDni[cantidad],"00");
    //strcpy(Memoria->pagoFecha[cantidad],"00");
    //printf("\nTerminó de escribir en memoria compartida\n");

    //Termine de escribir en memoria compartida
    sem_post(semutex); //Hago un V() al mutex
    sem_post(sesocios); //Hago un V() al semaforo de asistencia. Así podré darle el paso

    //Cierro los semaforos
    sem_close(semutex);
    sem_close(seasistencia);
    sem_close(sesocios);

    //Cierro el archivo
    fclose(archivoAsistencia);
    //printf("\nRecursos cerrados\n");
}

