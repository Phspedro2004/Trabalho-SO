// C program to implement one side of FIFO
// This side reads first, then reads
//https://www.geeksforgeeks.org/named-pipe-fifo-example-c-program/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "Banco.h"

#define TAM 100

Registro banco[TAM];
int cont = 0;
pthread_mutex_t m_banco = PTHREAD_MUTEX_INITIALIZER;


void *processar(void *arg){
    char *requisicao = (char *)arg;
    printf("Requisicao Recebida: %s\n", requisicao);
    pthread_mutex_lock(&m_banco);

    int op;
    sscanf(requisicao, "%d", &op);

    switch (op)
    {
    case 1:
        int id;
        char nome[50];
        if(sscanf(requisicao, "1 id=%d nome='%49[^']'", &id, nome) == 2){
            inserir(id,nome);
        }
        else{
            printf("comando INSERT invalido.\n");
        }
        break;
    
    default:
        break;
    }

}

int main()
{
	int fd1;

	// FIFO file path
	char * myfifo = "/tmp/myfifo";
	mkfifo(myfifo, 0666);
	char buffer[256];

	while (1)
	{
		// First open in read only and read
		fd1 = open(myfifo,O_RDONLY);
        if(fd1 == -1) continue; // caso nao consia entrar no pipe ele sai do loop
        
		while (read(fd1, buffer, sizeof(buffer)) > 0){
            char *req = strdup(buffer);
            pthread_t thread;
            pthread_create(&thread, NULL, processar, req);
            pthread_detatch(thread);
        }
        

		close(fd1);
	}
	return 0;
}
