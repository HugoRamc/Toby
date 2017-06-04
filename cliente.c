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

#define READ_TO_BUFFER 0


int puerto;
char const* ip;

typedef struct{//colocamos un espacio de mas por le terminador de cadena
	int flag;
	char compania[11]; 
	char operacion[160];
	char usuario[20];	
}Zona;

void *consumidores(void *argv){

	int sockid;
	struct sockaddr_in servidor;
	//paso 0 creacion del socket
	socket(AF_INET,SOCK_STREAM,0);

	if((sockid = socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("El socket no se pudo crear\n");	
		exit(1);
	}

	//paso 1 conexion con el servidor
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(puerto);
	servidor.sin_addr.s_addr = inet_addr(ip);

	char buffer[50];
	int tamrecv;
	char buffData[191];
	if(connect(sockid, (struct sockaddr *)&servidor,sizeof(servidor))==-1){
		perror("No se pudo conectar con el servidor");
		exit(1);
	}

	//paso 2 zona de comunicacion	
		//recibir un mensaje

		if((tamrecv = recv(sockid,(void*)buffer,sizeof(buffer),0))==-1){
			perror("No se recibió el mensaje");
			exit(1);

		}

		buffer[tamrecv] = '\0';

		printf("servidor: %s\n", buffer);
			//mandar un mensaje

		//if(strcmp(buffer,"conexion establecida :D")){

			for(int i=0;i<10;++i){
				//mandar la estructura
				

				strcpy(buffData,"AT&T unidos por le monde (saca las panochas PERROOO)$");
				strcat(buffData,"AT&T$");
				strcat(buffData,"PIT");

				if((send(sockid,(void*)buffData,strlen(buffData),READ_TO_BUFFER))==-1){
					perror("soy cliente, no mandé mensaje");
					exit(1);
				}

				if((tamrecv = recv(sockid,(void*)buffer,sizeof(buffer),0))==-1){
					perror("No se recibió el mensaje");
					exit(1);

				}
				buffer[tamrecv] = '\0';
				printf("servidor dice : %s\n", buffer);



				

			}

	close(sockid);
}


int main(int argc, char const *argv[])
{

	puerto = atoi(argv[1]);
	ip = argv[2];
	
	Zona memoria;
	pthread_t idHilo[4];

	

	
	for(int i=0;i<4;++i){

		pthread_create(&idHilo[i],NULL,consumidores,NULL);
	}

	for(int i=0;i<4;++i){
		pthread_join(idHilo[i],NULL);

	}


	
	printf("Conexion finalizada\n");



	return 0;
}