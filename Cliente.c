// C program to implement one side of FIFO
// This side writes first, then reads
//https://www.geeksforgeeks.org/named-pipe-fifo-example-c-program/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
 
int main()
{
    int fd;
    int op;
    int loop = 1;
    int ID;

    // FIFO file path
    char * myfifo = "/tmp/myfifo";

    char nome[50];
    char buffer[256];

 
    while (loop)
    {
        printf("== Menu ==\n");
        printf("1. Inserir\n");
        printf("2. Remover\n");
        printf("3. Select\n");
        printf("4. Atualizar\n");
        printf("=========\n");
        printf("0. Sair\n");
        printf("Insira sua opçao\n");
        scanf("%d", op);
        getchar();
        switch (op)
        {
        case 0:
            loop = 0;
            break;
        case 1:
            printf("insira o ID: ");
            scanf("%d", &ID);
            getchar();
            printf("Digite o nome");
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = '\0';
            snprintf(buffer,sizeof(buffer),"1 id=%d nome='%s'", ID, nome);
            break;
        default:
            break;
        }
        
    fd = open(myfifo, O_WRONLY);
    write(fd,buffer, strlen(buffer) + 1);
    close(fd);
    }

    return 0;
}