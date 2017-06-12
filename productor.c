#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define TAM_MEMORIA 4

int semid_llamadas;
int semid_mensajes;
int shmid_llamadas;
int shmid_mensajes;

typedef struct{
	int flag;
	char operacion[161];
	char compania[11];
	char usuario[21];
}Zona;

//Zona *Memoria;

Zona mem_llamadas[4];
Zona mem_mensajes[4];

union semun{
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    unsigned short int *array; /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
};

void init_semaphores(unsigned short int* sem_array,int semid){
    union semun semopts;
    semopts.array = sem_array;

	if(semctl(semid,0,SETALL,semopts) == -1){
		perror("semctl");
		exit(1);
	}
}

void linkmemory(Zona* memoria,int shmid){
  //Se enlaza el arreglo de zonas a la memoria creada
	  if((memoria = (Zona*)shmat(shmid,(Zona*) 0,SHM_EXEC)) == (void *)-1) { 
	      perror("shmat");
	      exit(1);
	  }
}

void llamadas(int idusuario){
	if(idusuario == 0){
		printf("Yo soy Pit, esta es una llamadas\n");
	}
	else if(idusuario == 1){
		printf("Yo soy Hugue, este es un llamadas\n");
	}
	else{
		printf("Yo soy Alonso, este es un llamadas\n");
	}
}

void mensajes(int idusuario){
	if(idusuario == 0){
		printf("Yo soy Pit, este es un mensaje\n");
	}
	else if(idusuario == 1){
		printf("Yo soy Hugue, este es un mensaje\n");
	}
	else{
		printf("Yo soy Alonso, este es un mensaje\n");
	}
}

//Funcion que realizara el productor
void *createproductors(void *arg){
	int idusuario = *((int*)arg);
	llamadas(idusuario);
	mensajes(idusuario);
}

int main(int argc, char const *argv[]){
	//Creacion de los semaforos  
	key_t key_llamadas;
	key_t key_mensajes;

	unsigned short int  sem_arrayllamadas[10] = {1,1,1,1,1,0,0,0,0,0};
	unsigned short int 	sem_arraymensajes[10] = {1,1,1,1,1,0,0,0,0,0};


	//Creacion de una llave unica ligada al archvo especificado
	if((key_llamadas = ftok("/bin/ls",'l')) == -1){
		perror("Error al establecer la llave");
		exit(1);
	}

	if((key_mensajes = ftok("/bin/dir",'m')) == -1){
		perror("Error al establecer la llave");
		exit(1);
	}


	//Creamos un SET de 10 semaforos para las llamadas
	if((semid_llamadas = semget(key_llamadas,10,0666 | IPC_CREAT | IPC_EXCL)) == -1){
		if((semid_llamadas = semget(key_llamadas,10,0666))==-1){
			perror("semget");	
			exit(1);	
		}
		else{
			printf("Productor: Me ligue exitosamente a los semaforos de las llamadas\n");

		}
	}
	else{
		//Inicializamos los semaforos
		printf("Productor: Cree exitosamente los semaforos de las llamadas\n");
		init_semaphores(sem_arrayllamadas,semid_llamadas);
	}

	//Creamos un SET de 10 semaforos para los mensajes
	if((semid_mensajes = semget(key_mensajes,10,0666 | IPC_CREAT | IPC_EXCL)) == -1){
		if((semid_mensajes = semget(key_mensajes,10,0666))==-1){
			perror("semget");	
			exit(1);	
		}
		else{
			printf("Productor: Me ligue exitosamente a los semaforos de los mensajes\n");

		}
	}
	else{
		//Inicializamos los semaforos
		printf("Productor: Cree exitosamente los semaforos de los mensajes\n");
		init_semaphores(sem_arraymensajes,semid_mensajes);
	}


  //Creacion de la memoria compartida para llamadas
  if((shmid_llamadas = shmget(key_llamadas,sizeof(Zona)*TAM_MEMORIA,IPC_CREAT|IPC_EXCL|0666)) == -1){
      /* El segmento ya existe - abrimos como consumidor*/
      if((shmid_llamadas = shmget(key_llamadas,sizeof(Zona)*TAM_MEMORIA,0)) == -1){
	       perror("shmget");
	       exit(1);
      }
      else{
        printf("Productor: Me ligue exitosamente a la memoria de las llamadas\n");
      }
  }
  else{
    printf("Productor: Cree exitosamente la memoria de las llamadas\n");
  }

  //Creacion de la memoria compartida para mensajes
  if((shmid_mensajes = shmget(key_mensajes,sizeof(Zona)*TAM_MEMORIA,IPC_CREAT|IPC_EXCL|0666)) == -1){
      /* El segmento ya existe - abrimos como consumidor*/
      if((shmid_mensajes = shmget(key_mensajes,sizeof(Zona)*TAM_MEMORIA,0)) == -1){
	       perror("shmget");
	       exit(1);
      }
      else{
        printf("Productor: Me ligue exitosamente a la memoria de los mensajes\n");
      }
  }
  else{
    printf("Productor: Cree exitosamente la memoria de los mensajes\n");
  }

  linkmemory(mem_llamadas,shmid_llamadas);
  linkmemory(mem_mensajes,shmid_mensajes);

	//Creacion de los hilos productores (Pit y Hugue)
	pthread_t productores[3];
	int param[3];

	for(int i=0;i<3;i++){
		param[i] = i;
		if((pthread_create(&productores[i],NULL,createproductors,(void*)&param[i])) != 0){
			perror("pthread_create");
			exit(1);
		}
	}

	for(int j=0;j<3;j++){
		if((pthread_join(productores[j],NULL)) == -1){
			perror("pthread_join");
			exit(1);
		}
	}

	return 0;
}