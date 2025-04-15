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

/// Total de registros
/// Banco[tam]


Registro banco[TAM];
int Total_Registros = 0;
pthread_mutex_t m_banco = PTHREAD_MUTEX_INITIALIZER;

void Inserir(int ID, char *Nome) {
    if (Total_Registros < TAM) {
        banco[Total_Registros].id = ID;
        strncpy(banco[Total_Registros].nome, Nome, 49);
        banco[Total_Registros].nome[49] = '\0'; // garantir término
        Total_Registros++;
        printf("[INSERIDO] id=%d nome=%s\n", ID, Nome);
    } else {
        printf("[ERRO] banco cheio\n");
    }
}

void Remover(int ID) {
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            for (int j = i; j < Total_Registros - 1; j++) {
                banco[j] = banco[j + 1];
            }
            Total_Registros--;
            printf("[REMOVIDO] id=%d\n", ID);
            return;
        }
    }
    if (Total_Registros == 0)
        printf("[ERRO] banco vazio\n");
    else
        printf("[ERRO] id=%d nao encontrado\n", ID);
}

void SelectId(int ID) {
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            printf("[Retornando] id=%d nome=%s\n", banco[i].id, banco[i].nome);
            return;
        }
    }
    if (Total_Registros == 0){
        printf(" Banco vazio\n");
    }
    else{
        printf("[ERRO] id=%d nao encontrado\n", ID);
    }
}

void SelectNome(char *Nome) {
    for (int i = 0; i < Total_Registros; i++) {
        if (strcmp(banco[i].nome, Nome) == 0) {
            printf("[Retornando] id=%d nome=%s\n", banco[i].id, banco[i].nome);
            return;
        }
    }
    if (Total_Registros == 0){
        printf(" Banco vazio\n");
    }
    else{
        printf("[ERRO] nome=%s nao encontrado\n", Nome);
    }
}

void Atualizar(int ID, char *Nome){
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            strncpy(banco[i].nome, Nome, 49);
            banco[i].nome[49] = '\0'; // garantir término
            printf("[ATUALIZADO] id=%d nome=%s\n", ID, Nome);
            return;
        }
    }
}

void Listar() {
    printf("[LISTA]\n");
    if (Total_Registros == 0) {
        printf(" Banco vazio\n");
        return;
    }
    for (int i = 0; i < Total_Registros; i++) {
        printf("id=%d nome=%s\n", banco[i].id, banco[i].nome);
    }
}

void *processar(void *arg) {
    char *requisicao = (char *)arg;
    printf("Requisicao Recebida: %s\n", requisicao);

    int OPC, ID;
    char Nome[50];

    if (sscanf(requisicao, "%d", &OPC) != 1) {
        printf("[ERRO] comando invalido.\n");
        free(requisicao);
        return NULL;
    }

    switch (OPC) {
        case 1: {
            if (sscanf(requisicao, "1 id=%d nome='%49[^']'", &ID, Nome) == 2) {
                printf("[DEBUG] Inserindo: id=%d, nome=%s\n", ID, Nome);
                pthread_mutex_lock(&m_banco);
                Inserir(ID, Nome);
                pthread_mutex_unlock(&m_banco);
            } else {
                printf("[ERRO] comando INSERT invalido.\n");
            }
            break;
        }
        case 2:
            if (sscanf(requisicao, "2 id=%d", &ID) == 1) {
                printf("[DEBUG] Removendo: id=%d\n", ID);
                pthread_mutex_lock(&m_banco);
                Remover(ID);
                pthread_mutex_unlock(&m_banco);
            } else {
                printf("[ERRO] comando REMOVE invalido.\n");
            }
            break;
        case 3:
            if (sscanf(requisicao, "3 id=%d", &ID) == 1) {
                printf("[DEBUG] Selecionando: id=%d\n", ID);
                pthread_mutex_lock(&m_banco);
                SelectId(ID);
                pthread_mutex_unlock(&m_banco);
            } else if (sscanf(requisicao, "3 nome='%49[^']'", Nome) == 1) {
                printf("[DEBUG] Selecionando: nome=%s\n", Nome);
                pthread_mutex_lock(&m_banco);
                SelectNome(Nome);
                pthread_mutex_unlock(&m_banco);
            } else {
                printf("[ERRO] comando SELECT invalido.\n");
            }
            break;
        case 4:
            if (sscanf(requisicao, "4 id=%d nome='%49[^']'", &ID, Nome) == 2) {
                printf("[DEBUG] Atualizando: id=%d, nome=%s\n", ID, Nome);
                pthread_mutex_lock(&m_banco);
                Atualizar(ID, Nome);
                pthread_mutex_unlock(&m_banco);
            } else {
                printf("[ERRO] formato invalido de comando.\n");
            }
            break;
        case 5:
            printf("[DEBUG] Listando registros\n");
            pthread_mutex_lock(&m_banco);
            Listar();
            pthread_mutex_unlock(&m_banco);
            break;
        default:
            printf("[ERRO] comando desconhecido: %d\n", OPC);
            break;
    }

    free(requisicao);
    return NULL;
}

int main() {
    int fd1;
    char *myfifo = "/tmp/myfifo";
    mkfifo(myfifo, 0666);
    char buffer[256];

    while (1) {
        fd1 = open(myfifo, O_RDONLY);
        if (fd1 == -1) continue;

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_read = read(fd1, buffer, sizeof(buffer));
            if (bytes_read <= 0) break;

            char *req = strdup(buffer);
            pthread_t thread;
            pthread_create(&thread, NULL, processar, req);
            pthread_detach(thread);
        }

        close(fd1);
    }

    return 0;
}
