#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <error.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

int semid;
int shmid;

union semun{
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    ushort *array;          /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
    void *__pad;
};

typedef struct{//colocamos un espacio de mas por le terminador de cadena
	int flag;
	char compania[11]; 
	char operacion[160];
	char usuario[20];	
}Zona;

Zona *Memoria;

void init_semaphore(unsigned short int* sem_array){
    union semun semopts;

    semopts.array =  sem_array;

	if(semctl(semid,0,SETALL,semopts) == -1){
		perror("semctl");
		exit(1);
	}
}

int CreaLigaMemoria(key_t key){ 

  //Verifica si el segmento de memoria existe (IPC_CREAT|IPC_EXCL)
  if((shmid = shmget(key,sizeof(Zona),IPC_CREAT|IPC_EXCL|0666)) == -1){
      /* El segmento existe - abrir como cliente */
      if((shmid = shmget(key,sizeof(Zona),0)) == -1){
	       perror("shmget");
	       exit(1);
      }
      else{
        printf("Yo soy el productor y me ligue a la memoria\n");
      }
  }
  else{
    printf("Yo soy el productor y cree la memoria\n");
  }

  //se ata a la zona de memoria creada
  if((Memoria = (Zona*)shmat(shmid,(Zona*) 0,0)) == (void *)-1) { 
      perror("shmat");
      exit(1);
  }
  //printf("%s\n",Memoria); //lee en memoria compartida

  return shmid; //regresa el identificador de la memoria
}


void *llamadas(void *argv){
	int idUsu = *((int*)argv);


	//xprintf("val 2N  = %d\n",idUsu);

	if(idUsu == 0){
		printf("Yo soy la llamada que hace pit\n");

	}else{
		printf("Yo soy la llamada que hace Hug\n");
	}

}

void *mensajes(void *argv){
	int idUsu = *((int*)argv);


	//printf("val 2N  = %d\n",idUsu);
	if(idUsu == 0){
		printf("Yo soy el mensaje que hace pit\n");

	}else{
		printf("Yo soy el mensaje que hace Hug\n");
	}

}

//funcion que realizará el productor
void *createProductors(void *argv){
	int idUsuario = *((int*)argv);

	//printf("val 1N  = %d\n",idUsuario);

	pthread_t operaciones[2];
	
	pthread_create(&operaciones[0],NULL,llamadas,(void *)&idUsuario);
	pthread_create(&operaciones[1],NULL,mensajes,(void *)&idUsuario);

	

	for(int i=0;i<2;++i){
		pthread_join(operaciones[i],NULL);
	}



	
}



int main(int argc, char const *argv[])
{
	//creacion de los semaforos
	key_t key;//lave para los semaforos
	unsigned short int sem_array[20]={1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0};

	if((key = ftok("/bin/ls",'k'))==-1){
		perror("error en la creacion de la memoria compartida");// creacion de una llave unica ligada al archivo especificado
		exit(1);
	}	
	//creacion de un set de 20 semaforos 

	if((semid = semget(key,20,0666 |IPC_CREAT | IPC_EXCL))==-1){//arreglo de 20 semáforos
		if((semid = semget(key,10,0666))==-1){
			perror("erorr en la creacion de los semaforos de las llamadas");
			exit(1);
		}else
		{
			printf("Productor: (llamadas) Me ligue exitosamente a los semaforos\n");
		}
	}else
	{
		//creamos los semaforos y luego los inicializamos
		printf("Productor: (llamadas) Cree exitosamente los semaforos\n");
		init_semaphore(sem_array);

		
	}

	//creacion de la zona de memoria

	if((shmid = CreaLigaMemoria(key))== -1){
		perror("Error en la creacion de la memoria");
		exit(1);
	}


	//creacion de los hilos productores de llamadas y mensajes pit y hugo
	pthread_t productores[2];
	int param[2];

	for(int i=0;i<2;++i){
		param[i] = i;

		pthread_create(&productores[i],NULL,createProductors,(void *)&param[i]);

	}

	for(int i=0;i<2;++i){
		pthread_join(productores[i],NULL);
	}





	return 0;
}