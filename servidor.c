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


/*
FILE *Hugue_llamadas;
FILE *Hugue_mensajes;
FILE *Pit_llamadas;
FILE *Pit_mensajes;
FILE *Alonso_llamadas;
FILE *Alonso_mensajes;
*/

FILE *llamadas;
FILE *mensajes;

//void createfiles();

void *acepta_conexion(void *arg){
	int *canal_id=(int *)arg;
	unsigned char buffermensaje[30];
	unsigned char bufferdata[191];
	unsigned char *arraydata[4];
	unsigned char *p;
	int index;
	int tamrecv;
	
	strcpy(buffermensaje,"Conexion establecida...");
	if(send(*canal_id,(void*)buffermensaje,strlen(buffermensaje),WRITE_TO_BUFFER)==-1){
		perror("No se pudo enviar el mensaje");
		exit(1);
	}
	strcpy(buffermensaje,"Mensaje recibido...");

	while(1){
		if((tamrecv = recv(*canal_id,(void*)bufferdata,sizeof(bufferdata),READ_TO_BUFFER))==-1){
			perror("No se recibio el mensaje");
			exit(1);
		}

		if(tamrecv == 0){
			break;
		}

		p = strtok(bufferdata,"$");

		index=0;
		while(p != NULL){
			arraydata[index++] = p;
			p = strtok(NULL,"$");
		}


		lock(0);
			if(strcmp(arraydata[0],"M")==0){
				if((mensajes = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registromensajes.txt","a")) == NULL){
				    perror("fopen\n");
				    exit(1);
				}
				printf("Cliente: %s\n",bufferdata);
				fputs("Usuario: ",mensajes);
				fputs(arraydata[3],mensajes);
				fputs("\tCompania: ",mensajes);
				fputs(arraydata[2],mensajes);
				fputs("\tOperacion: ",mensajes);
				fputs(arraydata[1],mensajes);
				fputs("\n",mensajes);
				fclose(mensajes);
			}
			else if(strcmp(arraydata[0],"L")==0){
				if((llamadas = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registrollamadas.txt","a")) == NULL){
				    perror("fopen\n");
				    exit(1);
				}
				printf("Cliente: %s\n",bufferdata);
				fputs("Usuario: ",llamadas);
				fputs(arraydata[3],llamadas);
				fputs("\tCompania: ",llamadas);
				fputs(arraydata[2],llamadas);
				fputs("\tOperacion: ",llamadas);
				fputs(arraydata[1],llamadas);
				fputs("\n",llamadas);
				fclose(llamadas);
			}
			else{
				printf("Error en la información\n");
			}						
		open(0);

		if(send(*canal_id,(void*)buffermensaje,strlen(buffermensaje),WRITE_TO_BUFFER)==-1){
			perror("No se pudo enviar el mensaje");
			exit(1);
		}

		printf("Se recibio el mensaje\n");
	}		

	close(*canal_id);
	free(canal_id);
	printf("Conexión Finalizada\n");
}

int main(int argc, char const *argv[]){
	int puerto = atoi (argv[1]);
	int sock_id;
	int *canal_id;
	int tam;
	struct sockaddr_in servidor,cliente;
	pthread_t *id_hilo;
	key_t key_archivos;

	unsigned short int sem_arrayarchivos[6] = {1,1,1,1,1,1};

	//Creacion de una llave unica ligada al archvo especificado
	if((key_archivos = ftok("/bin/bash",'a')) == -1){
		perror("Error al establecer la llave");
		exit(1);
	}

	//Creamos un SET de 10 semaforos para las llamadas
	if((semid_archivos = semget(key_archivos,6,0666 | IPC_CREAT | IPC_EXCL)) == -1){
		if((semid_archivos = semget(key_archivos,6,0666))==-1){
			perror("semget");	
			exit(1);	
		}
		else{
			printf("Servidor: Me ligue exitosamente a los semaforos de los archivos\n");
		}
	}
	else{
		//Inicializamos los semaforos
		printf("Servidor: Cree exitosamente los semaforos de los archivos\n");
		init_semaphores(sem_arrayarchivos,semid_archivos);
	}



	if((sock_id = socket(AF_INET,SOCK_STREAM,SINGLE_PROTOCOL))==-1){
		perror("El socket no se pudo construir\n");
		exit(1);
	}

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(puerto);
	servidor.sin_addr.s_addr = INADDR_ANY;

	if(bind(sock_id,(struct sockaddr*)&servidor,sizeof(servidor)) == -1){
		perror("Error al publicar el servicio\n");
		exit(1);
	}

	if(listen(sock_id,4)==-1){
		perror("No se pudo establecer la escucha\n");
		exit(1);
	}

	//createfiles();
	if((llamadas = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registrollamadas.txt","w")) == NULL){
		perror("fopen\n");
		exit(1);
	}
	fclose(llamadas);


	if((mensajes = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registromensajes.txt","w")) == NULL){
		perror("fopen\n");
		exit(1);
	}
	fclose(mensajes);

	while(1){
		id_hilo= (pthread_t *)malloc(sizeof(pthread_t));
		canal_id=(int *)malloc(sizeof(int));
		tam = sizeof(cliente);
		if((*canal_id=accept(sock_id,(struct sockaddr*)&cliente,&tam))==-1){
			perror("No se pudo establecer la conexión con el cliente\n");
			exit(1);
		}
		pthread_create(id_hilo,NULL,acepta_conexion,(void *)canal_id);
		free(id_hilo);
	}

	return 0;
}

/*
void createfiles(){
	if((Hugue_llamadas = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registrollamHugue.txt","w")) == NULL){
    	perror("fopen\n");
    	exit(1);
    }

    fclose(Hugue_llamadas);

    if((Hugue_mensajes = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registromensjHugue.txt","w")) == NULL){
    	perror("fopen\n");
    	exit(1);
    }

    fclose(Hugue_mensajes);

    if((Pit_llamadas = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registrollamPit.txt","w")) == NULL){
    	perror("fopen\n");
    	exit(1);
    }

    fclose(Pit_llamadas);

    if((Pit_mensajes = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registromensjPit.txt","w")) == NULL){
    	perror("fopen\n");
    	exit(1);
    }

    fclose(Pit_mensajes);

    if((Alonso_llamadas = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registrollamAlonso.txt","w")) == NULL){
    	perror("fopen\n");
    	exit(1);
    }

    fclose(Alonso_llamadas);

    if((Alonso_mensajes = fopen("/home/ESCOM/Documents/Sistemas Operativos/Proyecto Final/registromensjAlonso.txt","w")) == NULL){
    	perror("fopen\n");
    	exit(1);
    }

    fclose(Alonso_mensajes);
}
*/
