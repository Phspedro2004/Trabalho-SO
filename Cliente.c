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
    int ID;
    int loop = 1;

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
        printf("5. Listar\n");
        printf("=========\n");
        printf("0. Sair\n");
        printf("Insira sua opçao\n");
        scanf("%d", &op);
        getchar();
        switch (op)
        {
        case 0:
            loop = 0;
            break;
        case 1: //código pra inserção, pede o ID e Nome envia pro servidor junto com a identificação da operação "1";
            printf("insira o ID: ");
            scanf("%d", &ID);
            getchar();
            printf("Digite o nome: ");
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = '\0';
            snprintf(buffer,sizeof(buffer),"1 id=%d nome='%s'", ID, nome);
            break;
        case 2: //código pra remoção, pede o ID e envia pro servidor o id junto com a identificação da operação "2";
            printf("Insira o ID que deseja Remover:");
            scanf("%d", &ID);
            getchar();
            snprintf(buffer,sizeof(buffer),"2 id=%d", ID); 
            break;
        case 3:
            printf("Deseja procurar por:\n");
            printf("1. ID\n");
            printf("2. Nome\n");
            scanf("%d", &op);
            getchar();
            if(op == 1){
                printf("Insira o ID que deseja Selecionar:");
                scanf("%d", &ID);
                getchar();
                snprintf(buffer,sizeof(buffer),"3 id=%d", ID);
            }
            else if(op == 2){
                printf("Digite o nome: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = '\0';
                snprintf(buffer,sizeof(buffer),"3 nome='%s'", nome);
            }
            else{
                printf("Opção inválida.\n");
            }
            break;
        case 4: 
            printf("Insira o ID que deseja Atualizar:");
            scanf("%d", &ID);
            getchar();
            printf("Digite o novo nome: ");
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = '\0';
            snprintf(buffer,sizeof(buffer),"4 id=%d nome='%s'", ID, nome);
            break;
        case 5: //código pra listar, envia pro servidor a identificação da operação "5";
            snprintf(buffer,sizeof(buffer),"5");
            break;
        default:
            break;
        }
        if(op != 0){
            fd = open(myfifo, O_WRONLY);
            write(fd,buffer, strlen(buffer) + 1);
            close(fd);
        }
    }
    printf("Cliente Encerrado");
    return 0;
}