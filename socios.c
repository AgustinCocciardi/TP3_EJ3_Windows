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
#include <sys/stat.h>
#include <fcntl.h>
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

typedef struct socio
{
    char nombre[20];
    char apellido[30];
    char dni[9];
    char deporte[9];
    char dia[10];
} estructuraSocios;

/*
# ALUMNOS GRUPO 8 - Trabajo Practico 3
# Ejercicio 3
# 40231779 - Cocciardi, Agustin
# 40078823 - Biscaia, Elías
# 40388978 - Varela, Daniel
# 37841788 - Sullca, Willian
# 38056215 - Aguilera, Erik 
*/

int main(int argc, char* argv[]){
    char *ayuda="-Help";    //Muestro la ayuda
    if (argc == 2 && strcmp(argv[1],ayuda) == 0) //Muestro la ayuda al usuario
    {
        printf("\nEste programa tiene la funcionalidad de leer un archivo de socios y recibir información a procesar.");
        printf("\nLo que debe hacer este programa es leer del archivo de socios y procesar los datos que recibirá de los procesos Pagos y Asistencia");
        printf("\nEste debe ser el primer proceso en correr");
        printf("\nEjemplo de ejecución: ./Principal");
        printf("\n");
        exit(3);
    }

    if (argc != 1)
    {
        puts("Exceso de parámetros. Escriba './Principal -Help' para tener ayuda");
        exit(4);
    }

    //Datos del club//
    char *futbolDia1 = "Lunes";
    char *futbolDia2 = "Miercoles";
    char *voleyDia1 = "Martes";
    char *voleyDia2 = "Jueves";
    char *basquetDia = "Viernes";
    char *natacionDia = "Sabado";
    int futbolPrecio = 1000;
    int voleyPrecio = 1200;
    int basquetPrecio = 1300;
    int natacionPrecio = 1800;

    //Abro el archivo y valido que haya podido abrirlo
    char *socios="socios.txt";

    FILE *archivoSocios;
    archivoSocios=fopen(socios,"r");

    if (archivoSocios == NULL)
    {
        printf("\nEl archivo no existe o no tiene permisos de lectura\n");
        exit(2);
    }

    //Me va a servir para leer del archivo
    char delimitador[]=";\n";
    char palabra[85];

    char *dni;
    char *nombre;
    char *apellido;
    char *nombreDeporte;
    char *diaAsistencia;
    
    int contador=0;
    while (feof(archivoSocios) == 0)
    {
        fgets(palabra,85,archivoSocios);
        contador++;
        /*nombre= strtok(palabra,delimitador);
        apellido= strtok(NULL,delimitador);
        dni= strtok(NULL,delimitador);
        nombreDeporte= strtok(NULL,delimitador);
        diaAsistencia= strtok(NULL,delimitador);
        printf("Nombre y apellido: %s %s\t DNI: %s\t Deporte y Dia: %s %s\n", nombre, apellido, dni,nombreDeporte,diaAsistencia);*/
    }
    rewind(archivoSocios);

    //printf("Hay %d registros en el archivo", contador);

    estructuraSocios sociosClub[contador];
    int i=0;
    while (feof(archivoSocios) == 0)
    {
        fgets(palabra,85,archivoSocios);
        nombre= strtok(palabra,delimitador);
        apellido= strtok(NULL,delimitador);
        dni= strtok(NULL,delimitador);
        nombreDeporte= strtok(NULL,delimitador);
        diaAsistencia= strtok(NULL,delimitador);
        strcpy(sociosClub[i].nombre,nombre);
        strcpy(sociosClub[i].apellido,apellido);
        strcpy(sociosClub[i].dni,dni);
        strcpy(sociosClub[i].deporte,nombreDeporte);
        strcpy(sociosClub[i].dia,diaAsistencia);
        i++;
    }


    printf("\nSocios del club\n");
    for (int i = 0; i < contador; i++)
    {
        printf("Nombre y apellido: %s %s\t DNI: %s\t Deporte y Dia: %s %s\n", sociosClub[i].nombre, sociosClub[i].apellido, sociosClub[i].dni, sociosClub[i].deporte, sociosClub[i].dia);
    }
    printf("\n");
    
    //Cierro el archivo
    fclose(archivoSocios);
    
    //Para esta parte, ya tengo los datos de mis socios en la estructura sociosClub. Voy a empezar a abrir los recursos compartidos

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
    sem_t *sesocios;

    
    //Creo e inicializo los semáforos
    semutex = sem_open("/mutex",O_CREAT|O_EXCL,0644,1);               //Inicializo el Mutex en 1
    sepagos = sem_open("/pagos",O_CREAT|O_EXCL,0644,1);               //Inicializo el semaforo Pagos en 1
    seasistencia = sem_open("/asistencia",O_CREAT|O_EXCL,0644,0);     //Inicializo el semaforo Asistencia en 0
    sesocios = sem_open("/socios",O_CREAT|O_EXCL,0644,0);             //Inicializo el semaforo Socios en 0
    
    //Hago un P() a mi semaforo de socios. Como está inicializado en 0, me quedo esperando a que el proceso asistencia haga un V()
    sem_wait(sesocios);
    sem_wait(semutex);  //Tomo el Mutex para leer de memoria compartida
    
    //printf("\nHasta acá llegó 1\n");

    //Calcular el monto mensual (Revisar descuentos)
    float montoEnero=0;
    float montoFebrero=0;
    float montoMarzo=0;
    float montoAbril=0;
    float montoMayo=0;
    float montoJunio=0;
    float montoJulio=0;
    float montoAgosto=0;
    float montoSeptiembre=0;
    float montoOctubre=0;
    float montoNoviembre=0;
    float montoDiciembre=0;
    int j=0;
    char auxDocDni[9];
    char auxFecha[11];
    char dniDelSocio[9];
    char *anio;
    char *mes;
    char *dia;
    char delim[3] = "-\0";
    char sport[10];
    float monto=0;
    int descuento;
    //printf("\n\n");
    //printf("\nMe preparo para entrar al while\n");
    while (strcmp(Memoria->pagoDni[j],"00") != 0)
    {
        descuento = 0;
        strcpy(auxDocDni,Memoria->pagoDni[j]);
        strcpy(auxFecha,Memoria->pagoFecha[j]);
        //printf("\nDNI: %s\tFecha: %s\n", auxDocDni, auxFecha);
        anio = strtok(auxFecha,delim);
        mes = strtok(NULL,delim);
        dia = strtok(NULL,delim);
        //printf("\nDia %s Mes %s Anio %s\n", dia, mes, anio);
        //printf("\nLlego hasta aca 11\n");
        
        for (int i = 0; i < contador; i++)
        {
            strcpy(dniDelSocio,sociosClub[i].dni);
            strcat(auxDocDni,"\0");
            strcat(dniDelSocio,"\0");
            int resultado = strcmp(auxDocDni,dniDelSocio);
            //LOGRÈ QUE LLEGUE HASTA ACÁ
            if ( resultado == 0)
            {
                strcpy(sport,sociosClub[i].deporte);
                break;
            }
        }
        //printf("\nLlego hasta aca 2\n");
        if (strcmp(dia,"01") == 0 || strcmp(dia,"02") == 0 || strcmp(dia,"03") == 0 || strcmp(dia,"04") == 0 || strcmp(dia,"05") == 0 || strcmp(dia,"06") == 0 || strcmp(dia,"07") == 0 || strcmp(dia,"08") == 0 || strcmp(dia,"09") == 0 || strcmp(dia,"10") == 0)
        {
            descuento=1;
        }
        if (strcmp(sport,"Futbol"))
        {
            monto=futbolPrecio;
        }
        if (strcmp(sport,"Voley"))
        {
            monto=voleyPrecio;
        }
        if (strcmp(sport,"Basquet"))
        {
            monto=basquetPrecio;
        }
        if (strcmp(sport,"Natacion"))
        {
            monto=natacionPrecio;
        }

        //printf("\nLlego hasta aca 3\n");
        if (descuento == 0)
        {
            monto-=(monto*10/100);
        }
        
        //printf("\nLlego hasta aca 4\n");
        //printf("\nMes: %s\n", mes);
        if (strcmp(mes,"01") == 0)
        {
            montoEnero+=monto;
        }
        else if (strcmp(mes,"02") == 0)
        {
            montoFebrero+=monto;
        }
        else if (strcmp(mes,"03") == 0)
        {
            montoMarzo+=monto;
        }
        else if (strcmp(mes,"04") == 0)
        {
            montoAbril+=monto;
        }
        else if (strcmp(mes,"05") == 0)
        {
            montoMayo+=monto;
        }
        else if (strcmp(mes,"06") == 0)
        {
            montoJunio+=monto;
        }
        else if (strcmp(mes,"07") == 0)
        {
            montoJulio+=monto;
        }
        else if (strcmp(mes,"08") == 0)
        {
            montoAgosto+=monto;
        }
        else if (strcmp(mes,"09") == 0)
        {
            montoSeptiembre+=monto;
        }
        else if (strcmp(mes,"10") == 0)
        {
            montoOctubre+=monto;
        }
        else if (strcmp(mes,"11") == 0)
        {
            montoNoviembre+=monto;
        }
        else
        {
            montoDiciembre+=monto;
        }
        j++;
    }

    printf("\nMontos cobrados por mes:\n");
    printf("\nMonto cobrado en enero: %.2f", montoEnero);
    printf("\nMonto cobrado en febrero: %.2f", montoFebrero);
    printf("\nMonto cobrado en marzo: %.2f", montoMarzo);
    printf("\nMonto cobrado en abril: %.2f", montoAbril);
    printf("\nMonto cobrado en mayo: %.2f", montoMayo);
    printf("\nMonto cobrado en junio: %.2f", montoJunio);
    printf("\nMonto cobrado en julio: %.2f", montoJulio);
    printf("\nMonto cobrado en agosto: %.2f", montoAgosto);
    printf("\nMonto cobrado en septiembre: %.2f", montoSeptiembre);
    printf("\nMonto cobrado en octubre: %.2f", montoOctubre);
    printf("\nMonto cobrado en noviembre: %.2f", montoNoviembre);
    printf("\nMonto cobrado en diciembre: %.2f", montoDiciembre);
    printf("\n\n\n");
    
    //printf("\nHasta acá llegó 2\n");
    //Detectar asociados que no pagaron cuota mensual
    printf("\nPagos de cuota mensual pendientes: \n");
    //LOGRE HACER QUE LLEGUE HASTA ACÁ
    char *socioActual;
    char *auxiliarFecha;
    int b;
    int enero, febrero, marzo, abril , mayo , junio , julio , agosto , septiembre , octubre , noviembre , diciembre ;
    //char *auxiliarAnio;
    //char *auxiliarMes;
    for (int i = 0; i < contador; i++)
    {
        //printf("\nLlega hasta acá 7\n");
        socioActual=sociosClub[i].dni;
        //strcpy(socioActual,sociosClub[i].dni);
        strcat(socioActual,"\0");
        //printf("\nLlega hasta acá 8\n");
        b=0;
        enero=0;
        febrero=0;
        marzo=0;
        abril=0;
        mayo=0;
        junio=0;
        julio=0;
        agosto=0;
        septiembre=0;
        octubre=0;
        noviembre=0;
        diciembre=0;   
        //printf("\nLlega hasta acá 9\n");     
        while (strcmp(Memoria->pagoDni[b],"00") != 0)
        {
            //printf("\nLlega hasta acá 10\n");
            /*strcpy(auxDocDni,Memoria->asistenciaDni[b]);
            strcat(auxDocDni,"\0");
            strcpy(auxFecha,Memoria->asistenciaDia[b]);
            strcat(auxFecha,"\0");*/
            //printf("\nSocioActual: '%s'\t DNI memoria: %s", socioActual, Memoria->pagoDni[b]);
            //sleep(2);
            if (strcmp(socioActual,Memoria->pagoDni[b]) == 0)
            {
                //printf("\nSon iguales\n");
                //sleep(2);
                strcpy(auxiliarFecha,Memoria->pagoFecha[b]);
                anio = strtok(auxiliarFecha,delim);
                mes = strtok(NULL,delim);
                if (strcmp(mes,"01") == 0)
                {
                    enero = 1;
                }
                else if (strcmp(mes,"02") == 0)
                {
                    febrero = 1;
                }
                else if (strcmp(mes,"03") == 0)
                {
                    marzo = 1;
                }
                else if (strcmp(mes,"04") == 0)
                {
                    abril = 1;
                }
                else if (strcmp(mes,"05") == 0)
                {
                    mayo = 1;
                }
                else if (strcmp(mes,"06") == 0)
                {
                    junio = 1;
                }
                else if (strcmp(mes,"07") == 0)
                {
                    julio = 1;
                }
                else if (strcmp(mes,"08") == 0)
                {
                    agosto = 1;
                }
                else if (strcmp(mes,"09") == 0)
                {
                    septiembre = 1;
                }
                else if (strcmp(mes,"10") == 0)
                {
                    octubre = 1;
                }
                else if (strcmp(mes,"11") == 0)
                {
                    noviembre = 1;
                }
                else
                {
                    diciembre = 1;
                }

            }
            
            b++;
        }
        if (enero == 0)
        {
            printf("El socio %s no abono el mes de enero\n", socioActual);
        }
        if (febrero == 0)
        {
            printf("El socio %s no abono el mes de febrero\n", socioActual);
        }
        if (marzo == 0)
        {
            printf("El socio %s no abono el mes de marzo\n", socioActual);
        }
        if (abril == 0)
        {
            printf("El socio %s no abono el mes de abril\n", socioActual);
        }
        if (mayo == 0)
        {
            printf("El socio %s no abono el mes de mayo\n", socioActual);
        }
        if (junio == 0)
        {
            printf("El socio %s no abono el mes de junio\n", socioActual);
        }
        if (julio == 0)
        {
            printf("El socio %s no abono el mes de julio\n", socioActual);
        }
        if (agosto == 0)
        {
            printf("El socio %s no abono el mes de agosto\n", socioActual);
        }
        if (septiembre == 0)
        {
            printf("El socio %s no abono el mes de septiembre\n", socioActual);
        }
        if (octubre == 0)
        {
            printf("El socio %s no abono el mes de octubre\n", socioActual);
        }
        if (noviembre == 0)
        {
            printf("El socio %s no abono el mes de noviembre\n", socioActual);
        }
        if (diciembre == 0)
        {
            printf("El socio %s no abono el mes de diciembre\n", socioActual);
        }
        printf("\n----------------\n");
        //sleep(2);
    }
    
    printf("\n\n\n");
    printf("\nAsistencias en dias que no corresponden: \n");
    //Detectar asistencias en dias que no corresponden
    //printf("\nHasta aca llego 10\n");
    int a=0;
    //char auxDni[9];
    //char auxDia[10];
    while (strcmp(Memoria->asistenciaDni[a],"") != 0)
    {
        //printf("\n'%s'\t'%s'\n",Memoria->asistenciaDni[a],Memoria->asistenciaDia[a]);
        //sleep(1);
        //strcpy(auxDni,Memoria->asistenciaDni[a]);
        //strcpy(auxDia,Memoria->asistenciaDia[a]);
        //printf("\n'%s'\t'%s'\n",auxDni,auxDia);
        for (int i = 0; i < contador; i++)
        {
            if (strcmp(Memoria->asistenciaDni[a],sociosClub[i].dni) == 0)
            {
                if (strcmp(sociosClub[i].deporte,"Futbol") == 0)
                {
                    if (strcmp(Memoria->asistenciaDia[a],"Lunes") == 0 || strcmp(Memoria->asistenciaDia[a],"Miercoles") == 0)
                    {
                        /*SKIP*/
                    }
                    else
                    {
                        printf("El socio %s asiste el dia %s cuando debería asistir el día 'Lunes' o el día 'Miercoles'\n",Memoria->asistenciaDni[a],Memoria->asistenciaDia[a]);
                    }
                }
                if (strcmp(sociosClub[i].deporte,"Voley") == 0)
                {
                    if (strcmp(Memoria->asistenciaDia[a],"Martes") == 0 || strcmp(Memoria->asistenciaDia[a],"Jueves") == 0)
                    {
                        /*SKIP*/
                    }
                    else
                    {
                        printf("El socio %s asiste el dia %s cuando debería asistir el día 'Martes' o el día 'Jueves'\n",Memoria->asistenciaDni[a],Memoria->asistenciaDia[a]);
                    }
                }
                if (strcmp(sociosClub[i].deporte,"Basquet") == 0)
                {
                    if (strcmp(Memoria->asistenciaDia[a],"Viernes") == 0)
                    {
                        /*SKIP*/
                    }
                    else
                    {
                        printf("El socio %s asiste el dia %s cuando debería asistir el día 'Viernes'\n",Memoria->asistenciaDni[a],Memoria->asistenciaDia[a]);
                    }
                }
                if (strcmp(sociosClub[i].deporte,"Natacion") == 0)
                {
                    if (strcmp(Memoria->asistenciaDia[a],"Sabado") == 0)
                    {
                        /*SKIP*/
                    }
                    else
                    {
                        printf("El socio %s asiste el dia %s cuando debería asistir el día 'Sabado'\n",Memoria->asistenciaDni[a],Memoria->asistenciaDia[a]);
                    }
                }
                break;
            }
        }
        
        a++;
    }
    
    printf("\n\n");
    //printf("\nTerminó de leer de memoria compartida\n");

    //Cierro los semaforos
    sem_close(semutex);
    sem_close(sepagos);
    sem_close(seasistencia);
    sem_close(sesocios);

    //Elimino los semaforos
    sem_unlink("/mutex");
    sem_unlink("/pagos");
    sem_unlink("/asistencia");
    sem_unlink("/socios");

    //printf("\nRecursos cerrados\n");
}

