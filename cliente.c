#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <error.h>
#include <pthread.h>

#define SINGLE_PROTOCOL 0
#define WRITE_TO_BUFFER 0
#define READ_TO_BUFFER 0

int puerto;
char const* ip;

typedef struct{
	int flag;
	unsigned char operacion[161];
	unsigned char compania[11];
	unsigned char usuario[21];
}Zona;

void *enviar(void* arg){
	int idconsumidor = *((int*)arg);
	int tamrecv;
	int sock_id;
	struct sockaddr_in servidor;
	unsigned char buffermensaje[30];
	unsigned char bufferdata[191];
	Zona memoria;

	if((sock_id = socket(AF_INET,SOCK_STREAM,SINGLE_PROTOCOL))==-1){
		perror("El socket no se pudo construir\n");
		exit(1);
	}

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(puerto);
	servidor.sin_addr.s_addr = inet_addr(ip);

	if(connect(sock_id,(struct sockaddr *)&servidor,sizeof(servidor)) == -1){
		perror ("No se pudo conectar");
		exit (1);
	}

	if((tamrecv = recv(sock_id,(void*)buffermensaje,sizeof(buffermensaje),READ_TO_BUFFER))==-1){
		perror("No se recibio el mensaje");
		exit(1);
	}
	buffermensaje[tamrecv]='\0';
	printf("Servidor: %s\n",buffermensaje);

	for(int i=0;i<60;i++){
		switch(idconsumidor){
		case 0:		
				strcpy(memoria.operacion,"M$AT&T unidos por el mundo");
				strcpy(memoria.compania,"$AT&T");
				strcpy(memoria.usuario,"$Hugue");
				break;
		case 1:
				strcpy(memoria.operacion,"M$Telcel es la red");
				strcpy(memoria.compania,"$Telcel");
				strcpy(memoria.usuario,"$Pit");
			break;
		case 2:
				strcpy(memoria.operacion,"M$Movistar unidos hacemos mas");
				strcpy(memoria.compania,"$Movistar");
				strcpy(memoria.usuario,"$Alonso");
			break;
		default:
				strcpy(memoria.operacion,"L$5557928900");
				strcpy(memoria.compania,"$Telcel");
				strcpy(memoria.usuario,"$Hugue");
			break;
	}

		strcpy(bufferdata,memoria.operacion);
		strcat(bufferdata,memoria.compania);
		strcat(bufferdata,memoria.usuario);

		printf("%s\n",bufferdata);

		if(send(sock_id,(void*)bufferdata,strlen(bufferdata),WRITE_TO_BUFFER)==-1){
			perror("No se pudo enviar el mensaje");
			exit(1);
		}

		if((tamrecv = recv(sock_id,(void*)buffermensaje,sizeof(buffermensaje),READ_TO_BUFFER))==-1){
			perror("No se recibio el mensaje");
			exit(1);
		}
		buffermensaje[tamrecv]='\0';
		printf("Servidor: %s\n",buffermensaje);

	}
	printf("Conexión Finalizada\n");
	close (sock_id);			
}

int main(int argc, char const *argv[]){
	puerto = 2000;
	ip = "127.0.0.1";
	pthread_t consumidores[4];
	int parametros[4];

	for(int i=0;i<4;i++){
		parametros[i] = i;
		if((pthread_create(&consumidores[i],NULL,(void*)enviar,&parametros[i]))!=0){
			perror("\nError creating thread");
			exit(1);
		}
	}

	for(int j=0;j<4;j++){
		if((pthread_join(consumidores[j],NULL))==-1){
			perror("\nError joining thread");
			exit(1);
		}
	}

	return 0;
}

