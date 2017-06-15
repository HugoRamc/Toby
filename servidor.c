#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <string.h>
#include <error.h>
#include <pthread.h>

#define SINGLE_PROTOCOL 0
#define WRITE_TO_BUFFER 0
#define READ_TO_BUFFER 0


int semid_archivos;
int *sock_client;
int iteraciones;
pthread_t *id_hilo;

FILE* Pitcalls;
FILE* Huguecalls;
FILE* Alonsocalls;
FILE* Pitmsgs;
FILE* Huguemsgs;
FILE* Alonsomsgs;

struct{
    unsigned short int sem_num;
    short int sem_op;
    short int sem_flg;
}sembuf;

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

void open(int sem_num){
	struct sembuf sem_open;
	sem_open.sem_num = sem_num;
    sem_open.sem_op = 1;
    sem_open.sem_flg = SEM_UNDO; //Free resource of the semaphore
    if (semop(semid_archivos,&sem_open,1) == -1) {
        perror("semop");
        exit(1);
    }
}

void lock(int sem_num){
	struct sembuf sem_lock;
	sem_lock.sem_num = sem_num;
    sem_lock.sem_op = -1;  //Block the calling process until the value of the semaphore is greater than or equal to the absolute value of sem_op.
    sem_lock.sem_flg = SEM_UNDO;

	if (semop(semid_archivos,&sem_lock,1) == -1) {
        perror("semop");
        exit(1);
    }
}

void createfiles(){
	if((Pitcalls = fopen("Pitcalls.txt","w")) == NULL){
		perror("Error al crear el archivo");
		exit(1);
	}

	fclose(Pitcalls);

	if((Huguecalls = fopen("Huguecalls.txt","w")) == NULL){
		perror("Error al crear el archivo");
		exit(1);
	}

	fclose(Huguecalls);

	if((Alonsocalls = fopen("Alonsocalls.txt","w")) == NULL){
		perror("Error al crear el archivo");
		exit(1);
	}

	fclose(Alonsocalls);

	if((Pitmsgs = fopen("Pitmsgs.txt","w")) == NULL){
		perror("Error al crear el archivo");
		exit(1);
	}

	fclose(Pitmsgs);

	if((Huguemsgs = fopen("Huguemsgs.txt","w")) == NULL){
		perror("Error al crear el archivo");
		exit(1);
	}

	fclose(Huguemsgs);

	if((Alonsomsgs = fopen("Alonsomsgs.txt","w")) == NULL){
		perror("Error al crear el archivo");
		exit(1);
	}

	fclose(Alonsomsgs);
}

void *acepta_conexion(void *arg){
	/*Inicializacion de varibales*/
  	int *canal_id = (int *)arg;
  	int index;
  	int i=0;
	int tamrecv;
	char nombre[7];
	unsigned char bufferdata[191];
	unsigned char datoarchivo[200];
	unsigned char buffermensaje[31];
	unsigned char *arraydata[4];
	unsigned char *p;

	free(sock_client);
	printf("Conexión establecida\n");
	strcpy(buffermensaje,"Mensaje recibido y procesado");

	do{
		
		if((tamrecv = recv(*canal_id,(void*)bufferdata,sizeof(bufferdata),READ_TO_BUFFER))==-1){
			perror("No se recibio el mensaje");
			exit(1);
		}

		bufferdata[tamrecv]='\0';
		printf("Cliente: %s\n",bufferdata);

			
		p = strtok(bufferdata,"$");
		index=0;
		while(p != NULL){
			arraydata[index++] = p;
			p = strtok(NULL,"$");
		}

		if(strcmp(arraydata[0],"M") == 0){
			if(strcmp(arraydata[3],"Pit") == 0){
				if((Pitmsgs = fopen("Pitmsgs.txt","a")) == NULL){
					perror("Error al crear el archivo");
					exit(1);
				}

				strcpy(datoarchivo,"Compania: ");
				strcat(datoarchivo,arraydata[2]);
				strcat(datoarchivo,"	Mensaje: ");
				strcat(datoarchivo,arraydata[1]);
				strcat(datoarchivo,"\n");

				fwrite(datoarchivo,sizeof(char),strlen(datoarchivo),Pitmsgs);
				fclose(Pitmsgs);
			}
			else if(strcmp(arraydata[3],"Hugue") == 0){
				if((Huguemsgs = fopen("Huguemsgs.txt","a")) == NULL){
					perror("Error al crear el archivo");
					exit(1);
				}

				strcpy(datoarchivo,"Compania: ");
				strcat(datoarchivo,arraydata[2]);
				strcat(datoarchivo,"	Mensaje: ");
				strcat(datoarchivo,arraydata[1]);
				strcat(datoarchivo,"\n");

				fwrite(datoarchivo,sizeof(char),strlen(datoarchivo),Huguemsgs);
				fclose(Huguemsgs);
			}
			else{
				if((Alonsomsgs = fopen("Alonsomsgs.txt","a")) == NULL){
					perror("Error al crear el archivo");
					exit(1);
				}

				strcpy(datoarchivo,"Compania: ");
				strcat(datoarchivo,arraydata[2]);
				strcat(datoarchivo,"	Mensaje: ");
				strcat(datoarchivo,arraydata[1]);
				strcat(datoarchivo,"\n");

				fwrite(datoarchivo,sizeof(char),strlen(datoarchivo),Alonsomsgs);
				fclose(Alonsomsgs);
			}
		}
		else{
			if(strcmp(arraydata[3],"Pit") ==0 ){
				if((Pitcalls = fopen("Pitcalls.txt","a")) == NULL){
					perror("Error al crear el archivo");
					exit(1);
				}

				strcpy(datoarchivo,"Compania: ");
				strcat(datoarchivo,arraydata[2]);
				strcat(datoarchivo,"	Numero: ");
				strcat(datoarchivo,arraydata[1]);
				strcat(datoarchivo,"\n");

				fwrite(datoarchivo,sizeof(char),strlen(datoarchivo),Pitcalls);
				fclose(Pitcalls);
				
			}
			else if(strcmp(arraydata[3],"Hugue") == 0){
				if((Huguecalls = fopen("Huguecalls.txt","a")) == NULL){
					perror("Error al crear el archivo");
					exit(1);
				}

				strcpy(datoarchivo,"Compania: ");
				strcat(datoarchivo,arraydata[2]);
				strcat(datoarchivo,"	Numero: ");
				strcat(datoarchivo,arraydata[1]);
				strcat(datoarchivo,"\n");

				fwrite(datoarchivo,sizeof(char),strlen(datoarchivo),Huguecalls);
				fclose(Huguecalls);
			}
			else{
				if((Alonsocalls = fopen("Alonsocalls.txt","a")) == NULL){
					perror("Error al crear el archivo");
					exit(1);
				}

				strcpy(datoarchivo,"Compania: ");
				strcat(datoarchivo,arraydata[2]);
				strcat(datoarchivo,"	Numero: ");
				strcat(datoarchivo,arraydata[1]);
				strcat(datoarchivo,"\n");

				fwrite(datoarchivo,sizeof(char),strlen(datoarchivo),Alonsocalls);
				fclose(Alonsocalls);
			}
		}

		if(send(*canal_id,(void*)buffermensaje,strlen(buffermensaje),WRITE_TO_BUFFER)==-1){
			perror("No se pudo enviar el mensaje");
			exit(1);
		}

		printf("Información procesada\n");
		i++;
	}while(i<(600*iteraciones));

	printf("Fin de la conexión\n");
}

int main(int argc, char const *argv[]){
	if(argc <2){
		printf("Faltan parametros...\n");
	}
	else{
		int puerto = atoi (argv[1]);
		iteraciones = atoi(argv[2]);
		int sock_id;
		int tam;
		int avilable = 1;
		struct sockaddr_in servidor,cliente;

		createfiles();

		if((sock_id = socket(AF_INET,SOCK_STREAM,SINGLE_PROTOCOL))==-1){
			perror("El socket no se pudo construir\n");
			exit(1);
		}

		servidor.sin_family = AF_INET;
		servidor.sin_port = htons(puerto);
		servidor.sin_addr.s_addr = INADDR_ANY;
		setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR, &avilable, sizeof(int));

		if(bind(sock_id,(struct sockaddr*)&servidor,sizeof(servidor)) == -1){
			perror("Error al publicar el servicio\n");
			exit(1);
		}

		if(listen(sock_id,4)==-1){
			perror("No se pudo establecer la escucha\n");
			exit(1);
		}

		while(1){
			setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR, &avilable, sizeof(int));
			id_hilo = (pthread_t *)malloc(sizeof(pthread_t));
			sock_client = (int *)malloc(sizeof(int));
			tam = sizeof(cliente);
			if((*sock_client=accept(sock_id,(struct sockaddr*)&cliente,&tam))==-1){
				perror("No se pudo establecer la conexión con el cliente\n");
				exit(1);
			}
			pthread_create(id_hilo,NULL,acepta_conexion,(void *)sock_client);
			free(id_hilo);
		}
		close(sock_id);
	}

	return 0;
}