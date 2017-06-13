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
#include <string.h>

#define TAM_MEMORIA 4

#define MAIN_PRODUCER 0
#define MAIN_SPOOLER 1
#define MEM_PRODUCER1 2
#define MEM_SPOOLER1 3
#define MEM_PRODUCER2 4
#define MEM_SPOOLER2 5
#define MEM_PRODUCER3 6
#define MEM_SPOOLER3 7
#define MEM_PRODUCER4 8
#define MEM_SPOOLER4 9



int semid_llamadas;
int shmid_llamadas;
int semid_mensajes;
int shmid_mensajes;

struct{
    unsigned short int sem_num;
    short int sem_op;
    short int sem_flg;
}sembuf;

typedef struct{
	int flag;
	char operacion[161];
	char compania[11];
	char usuario[21];
}Zona;

Zona* mem_llamadas;
Zona* mem_mensajes;

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

void open(int sem_num,int semid){
	struct sembuf sem_open;
	sem_open.sem_num = sem_num;
    sem_open.sem_op = 1;
    sem_open.sem_flg = SEM_UNDO; //Free resource of the semaphore
    if (semop(semid,&sem_open,1) == -1) {
        perror("semop");
        exit(1);
    }
}

void lock(int sem_num,int semid){
	struct sembuf sem_lock;
	sem_lock.sem_num = sem_num;
    sem_lock.sem_op = -1;  //Block the calling process until the value of the semaphore is greater than or equal to the absolute value of sem_op.
    sem_lock.sem_flg = SEM_UNDO;

	if (semop(semid,&sem_lock,1) == -1) {
        perror("semop");
        exit(1);
    }
}

void remove_semaphores(int semid){
	union semun semopts;
	if (semctl(semid,0,IPC_RMID,semopts) == -1) {
        perror("semctl");
        exit(1);
    }
    printf("Destruyo semaforos\n");
}

Zona* linkmemory(int shmid){
	Zona* memoria;
  //Se enlaza el arreglo de zonas a la memoria creada
	  if((memoria = (Zona*)shmat(shmid,(Zona*) 0,SHM_EXEC)) == (void *)-1) { 
	      perror("shmat");
	      exit(1);
	  }
	return memoria;
}

void destroymemory(int shmid,Zona* buffer){
  //Terminada de usar la memoria compartida se libera
  if(shmid != 0 || shmid != -1){ //si la memoria ya fue destruida solo se des asocia de ella
    shmdt (buffer);//des asocia la memoria compartida de la zona de datos de nuestro programa
    shmctl (shmid, IPC_RMID, (struct shmid_ds *)NULL);//destruye la zona de memoria compartida
    //printf("\nLa memoria compartida ha sido destruida OUT\n");
    printf("Destruyo la memoria\n");
  }
  else{
    shmdt(buffer);//desasocia la memoria compartida de la zona de datos de nuestro programa
    printf("Se Desligo de la memoria\n");
  }
}

void consumememorycalls(int indexmem){
	unsigned char bufferdata[191];
	strcpy(bufferdata,mem_llamadas[indexmem].operacion);
	strcat(bufferdata,mem_llamadas[indexmem].compania);
	strcat(bufferdata,mem_llamadas[indexmem].usuario);
	printf("%s\n",bufferdata);
}

void consumememorymensages(int indexmem){
	unsigned char bufferdata[191];
	strcpy(bufferdata,mem_mensajes[indexmem].operacion);
	strcat(bufferdata,mem_mensajes[indexmem].compania);
	strcat(bufferdata,mem_mensajes[indexmem].usuario);
	printf("%s\n",bufferdata);
}

void consumecalls(){
	int sem_memvalue;
	int loop;
		//Zonas Criticas
			lock(MAIN_SPOOLER,semid_llamadas);
				loop = 1;
				do{
					sem_memvalue = semctl(semid_llamadas,MEM_SPOOLER1,GETVAL,NULL);
					if(sem_memvalue!=0){
						open(MAIN_SPOOLER,semid_llamadas);
						//Zona de memoria 1
						lock(MEM_SPOOLER1,semid_llamadas);
						consumememorycalls(0);
						open(MEM_PRODUCER1,semid_llamadas);
						loop = 0;
					}
					else{
						sem_memvalue = semctl(semid_llamadas,MEM_SPOOLER2,GETVAL,NULL);
						if(sem_memvalue!=0){
							open(MAIN_SPOOLER,semid_llamadas);
							//Zona de memoria 2
							lock(MEM_SPOOLER2,semid_llamadas);
							consumememorycalls(1);
							open(MEM_PRODUCER2,semid_llamadas);
							loop = 0;
						}
						else{
							sem_memvalue = semctl(semid_llamadas,MEM_SPOOLER3,GETVAL,NULL);
							if(sem_memvalue!=0){
								open(MAIN_SPOOLER,semid_llamadas);
								//Zona de memoria 3
								lock(MEM_SPOOLER3,semid_llamadas);
								consumememorycalls(2);
								open(MEM_PRODUCER3,semid_llamadas);
								loop = 0;
							}
							else{
								sem_memvalue = semctl(semid_llamadas,MEM_SPOOLER4,GETVAL,NULL);
								if(sem_memvalue!=0){
									open(MAIN_SPOOLER,semid_llamadas);
									//Zona de memoria 4
									lock(MEM_SPOOLER4,semid_llamadas);
									consumememorycalls(3);
									open(MEM_PRODUCER4,semid_llamadas);
									loop = 0;
								}
							}
						}
					}	
				}while(loop);
}

void consumemensages(){
	int sem_memvalue;
	int loop;
		//Zonas Criticas
			lock(MAIN_SPOOLER,semid_mensajes);
				loop = 1;
				do{
					sem_memvalue = semctl(semid_mensajes,MEM_SPOOLER1,GETVAL,NULL);
					if(sem_memvalue!=0){
						open(MAIN_SPOOLER,semid_mensajes);
						//Zona de memoria 1
						lock(MEM_SPOOLER1,semid_mensajes);
						consumememorymensages(0);
						open(MEM_PRODUCER1,semid_mensajes);
						loop = 0;
					}
					else{
						sem_memvalue = semctl(semid_mensajes,MEM_SPOOLER2,GETVAL,NULL);
						if(sem_memvalue!=0){
							open(MAIN_SPOOLER,semid_mensajes);
							//Zona de memoria 2
							lock(MEM_SPOOLER2,semid_mensajes);
							consumememorymensages(1);
							open(MEM_PRODUCER2,semid_mensajes);
							loop = 0;
						}
						else{
							sem_memvalue = semctl(semid_mensajes,MEM_SPOOLER3,GETVAL,NULL);
							if(sem_memvalue!=0){
								open(MAIN_SPOOLER,semid_mensajes);
								//Zona de memoria 3
								lock(MEM_SPOOLER3,semid_mensajes);
								consumememorymensages(2);
								open(MEM_PRODUCER3,semid_mensajes);
								loop = 0;
							}
							else{
								sem_memvalue = semctl(semid_mensajes,MEM_SPOOLER4,GETVAL,NULL);
								if(sem_memvalue!=0){
									open(MAIN_SPOOLER,semid_mensajes);
									//Zona de memoria 4
									lock(MEM_SPOOLER4,semid_mensajes);
									consumememorymensages(3);
									open(MEM_PRODUCER4,semid_mensajes);
									loop = 0;
								}
							}
						}
					}	
				}while(loop);
}


void *consumedata(void *arg){
	int idconsumidor = *((int*)arg);
	for(int i=0;i<15;i++){
		if(idconsumidor < 2){
			consumecalls();
		}
		else{
			consumemensages();
		}
	}
}


int main(int argc, char const *argv[]){
	//Creacion de los semaforos  
	key_t key_llamadas;
	key_t key_mensajes;

	unsigned short int  sem_arrayllamadas[10] = {1,1,1,0,1,0,1,0,1,0};
	unsigned short int  sem_arraymensajes[10] = {1,1,1,0,1,0,1,0,1,0};

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
			printf("Consumidor: Me ligue exitosamente a los semaforos de las llamadas\n");

		}
	}
	else{
		//Inicializamos los semaforos
		printf("Consumidor: Cree exitosamente los semaforos de las llamadas\n");
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
	        printf("Consumidor: Me ligue exitosamente a la memoria de las llamadas\n");
	      }
	  }
	  else{
	    printf("Consumidor: Cree exitosamente la memoria de las llamadas\n");
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

	  mem_llamadas = linkmemory(shmid_llamadas);
	  mem_mensajes = linkmemory(shmid_mensajes);

	//Creacion de los hilos productores (Pit y Hugue)
	pthread_t consumidores[4];
	int param[4];

	for(int i=0;i<4;i++){
		param[i] = i;
		if((pthread_create(&consumidores[i],NULL,consumedata,(void*)&param[i]) != 0)){
			perror("pthread_create");
			exit(1);
		}
	}

	for(int j=0;j<4;j++){
		if((pthread_join(consumidores[j],NULL)) == -1){
			perror("pthread_join");
			exit(1);
		}
	}

	remove_semaphores(semid_llamadas);
	remove_semaphores(semid_mensajes);

	destroymemory(shmid_llamadas,mem_llamadas);
	destroymemory(shmid_mensajes,mem_mensajes);

	return 0;
}