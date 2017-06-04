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

#define SINGLE_PROTOCOL 0
#define WRITE_TO_BUFFER 0
#define READ_TO_BUFFER 0


void procesaTrama(char *ptrData){


}

void *aceptaConexion(void *argv){
	int *canal_id = (int *)argv;
	//comunicacion con los clientes
	char buffMensaje[30];
	char buffData[191];//tamaño definido por la estructrua
	int tamrecv;
	int cont = 0;
	
	//Creacion de los archivos para guardar los datos :D
	FILE *Pit;
	if((Pit = fopen("pit.txt","w"))==NULL){


	}
	//funcion send para sockets
	//funcion write -> se ve como archivo -> no es lo correcto, best practice usar send
	//send to para sockets udp
	

		strcpy(buffMensaje,"conexion establecida :D");
	
		if((send(*canal_id,(void*)buffMensaje,strlen(buffMensaje),WRITE_TO_BUFFER))==-1){
			perror("No se pudo mandar el mensaje");
			exit(1);
		}


		strcpy(buffMensaje,"manda la siguiente");
		//send regresa el numero de caracteres enviados


		//Paso 5 recibimos mensaje
	while(1){

		if((tamrecv = recv(*canal_id,(void*)buffData,sizeof(buffData),READ_TO_BUFFER))==-1){
			perror("No se recibió el mensaje");
			exit(1);

		}

		buffData[tamrecv] = '\0';

		printf("Trama %d %s\n",cont,buffData);

		//procesaTrama(buffData);

		fputs(buffData,Pit);
		fputs("\n",Pit);



		cont+=1;

		if((send(*canal_id,(void*)buffMensaje,strlen(buffMensaje),WRITE_TO_BUFFER))==-1){
			perror("No se pudo mandar el mensaje");
			exit(1);
		}

		if(tamrecv == 0){
			break;
		}
	}
	
	fclose(Pit);
	close(*canal_id);
	printf("Fin de la comunicacion\n");
	free(canal_id);
}


int main(int argc, char const *argv[])
{
	
	int puerto = atoi(argv[1]);
	int sockid;
	struct sockaddr_in servidor,cliente;
	int *canal_id;

	pthread_t *idhilo;

	//paso 0 creacion del socket
	socket(AF_INET,SOCK_STREAM,0);

	if((sockid = socket(AF_INET,SOCK_STREAM,SINGLE_PROTOCOL))==-1)
	{
		perror("El socket no se pudo crear\n");	
		exit(1);
	}

	servidor.sin_family = AF_INET;		//familia de la dirección
	servidor.sin_port = htons(puerto); //host to network short
	servidor.sin_addr.s_addr = INADDR_ANY;

	if((bind(sockid,((struct sockaddr*)&servidor),sizeof(servidor))==-1))
	{
		perror("Error en el bind del socket");
		exit(1);
	}

	//Paso 2 hacer la escucha de una solicitud de conexión
	if((listen(sockid,4))==-1){//4 es el numero maximo de clientes 
		perror("Error en la escucha \n");
		exit(1);
	}

	//Paso 3 aceptar la conexión
	cliente.sin_family = AF_INET;
	
	//accept regresa el identificador de canal 
	int tam;
	while(1){

		idhilo = (pthread_t *)malloc(sizeof(pthread_t));
		canal_id = (int *)malloc(sizeof(int));;
		tam = sizeof(cliente);
		if((*canal_id = accept(sockid,(struct sockaddr*)&cliente,&tam))==-1){
			perror("error en la creacion de la acep");

		}
		pthread_create(idhilo,NULL,aceptaConexion,(void *)canal_id);
		free(idhilo);
	}

	return 0;
}