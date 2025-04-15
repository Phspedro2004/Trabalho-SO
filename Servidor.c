#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "Banco.h"

#define THREAD_NUM 4         // Número de threads que processam as tarefas
#define MAX_TASKS 256        
#define TAM 100              
#define ARQUIVO_BANCO "banco.txt"  

//tarefas que vão ser processadas
typedef struct Task {
    char *requisicao;
} Task;

// Argumento para identificar a thread
typedef struct {
    int id_thread;
} ThreadArgs;

Task taskQueue[MAX_TASKS];       // Fila de tarefas
int taskCount = 0;               // Quantidade de tarefas na fila

// Mutex e condição para controlar o acesso à fila de tarefas
pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

// Banco de dados em memória
Registro banco[TAM];
int Total_Registros = 0;

// Mutex para proteger o acesso ao banco de dados
pthread_mutex_t m_banco = PTHREAD_MUTEX_INITIALIZER;

// Protótipos
void *processar(void *arg);

// Enfileira uma nova tarefa
void Enfileirar(Task task) {
    pthread_mutex_lock(&mutexQueue);

    if (taskCount < MAX_TASKS) {
        taskQueue[taskCount++] = task;
        pthread_cond_signal(&condQueue); // Notifica uma thread que tem trabalho novo
        printf("[FILA] Tarefa enfileirada: %s\n", task.requisicao);
    } else {
        printf("[ERRO] Fila de tarefas cheia! Tarefa descartada.\n");
        free(task.requisicao);  // Libera a memória para evitar vazamento
    }

    pthread_mutex_unlock(&mutexQueue);
}

// Retira uma tarefa da fila
Task DesenfileirarTask() {
    Task task;

    pthread_mutex_lock(&mutexQueue);
    while (taskCount == 0) {
        pthread_cond_wait(&condQueue, &mutexQueue); // Espera até chegar tarefa
    }

    task = taskQueue[0];

    // Reorganiza a fila (puxa todos um pra frente)
    for (int i = 0; i < taskCount - 1; i++) {
        taskQueue[i] = taskQueue[i + 1];
    }

    taskCount--;
    pthread_mutex_unlock(&mutexQueue);

    return task;
}

// Função executada por cada thread para processar tarefas
void* Desenfileirar(void* args) {
    ThreadArgs *info = (ThreadArgs *)args;
    int id = info->id_thread;
    free(info);

    while (1) {
        printf("[THREAD %d] Aguardando tarefa...\n", id);
        Task task = DesenfileirarTask();
        printf("[THREAD %d] Processando: %s\n", id, task.requisicao);
        processar(task.requisicao);  // Libera a memória ao final
    }
}

// Salva o conteúdo do banco de dados no arquivo
void SalvarTxt() {
    FILE *fp = fopen(ARQUIVO_BANCO, "w");
    if (!fp) {
        perror("[ERRO] Falha ao abrir o arquivo para salvar");
        return;
    }

    for (int i = 0; i < Total_Registros; i++) {
        fprintf(fp, "%d;%s\n", banco[i].id, banco[i].nome);
    }

    fclose(fp);
}

// Carrega o conteúdo do banco de dados a partir do arquivo
void CarregarTxt() {
    FILE *fp = fopen(ARQUIVO_BANCO, "r");
    if (!fp) {
        printf("[INFO] Arquivo do banco não encontrado. Iniciando banco vazio.\n");
        return;
    }

    Total_Registros = 0;
    while (fscanf(fp, "%d;%49[^\n]\n", &banco[Total_Registros].id, banco[Total_Registros].nome) == 2) {
        if (++Total_Registros >= TAM) break;
    }

    fclose(fp);
}

// Função que insere um novo registro no banco
void Inserir(int ID, char *Nome) {
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            printf("[ERRO] ID %d já existe!\n", ID);
            return;
        }
    }

    if (Total_Registros < TAM) {
        banco[Total_Registros].id = ID;
        strncpy(banco[Total_Registros].nome, Nome, 49);
        banco[Total_Registros].nome[49] = '\0';
        Total_Registros++;
        SalvarTxt();
        printf("[SUCESSO] Inserido: id=%d nome=%s\n", ID, Nome);
    } else {
        printf("[ERRO] Banco cheio!\n");
    }
}

// Remove um registro com base no ID
void Remover(int ID) {
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            for (int j = i; j < Total_Registros - 1; j++) {
                banco[j] = banco[j + 1];
            }
            Total_Registros--;
            printf("[SUCESSO] Removido: id=%d\n", ID);
            return;
        }
    }

    printf("[ERRO] ID %d não encontrado.\n", ID);
}

// Busca um registro por ID
void SelectId(int ID) {
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            printf("[RESULTADO] id=%d nome=%s\n", banco[i].id, banco[i].nome);
            return;
        }
    }

    printf("[ERRO] ID %d não encontrado.\n", ID);
}

// Busca um registro por nome
void SelectNome(char *Nome) {
    for (int i = 0; i < Total_Registros; i++) {
        if (strcmp(banco[i].nome, Nome) == 0) {
            printf("[RESULTADO] id=%d nome=%s\n", banco[i].id, banco[i].nome);
            return;
        }
    }

    printf("[ERRO] Nome '%s' não encontrado.\n", Nome);
}

// Atualiza o nome de um registro com base no ID
void Atualizar(int ID, char *Nome) {
    for (int i = 0; i < Total_Registros; i++) {
        if (banco[i].id == ID) {
            strncpy(banco[i].nome, Nome, 49);
            banco[i].nome[49] = '\0';
            printf("[SUCESSO] Atualizado: id=%d nome=%s\n", ID, Nome);
            return;
        }
    }

    printf("[ERRO] ID %d não encontrado para atualização.\n", ID);
}

// Mostra todos os registros do banco
void Listar() {
    printf("[LISTAGEM COMPLETA]\n");
    for (int i = 0; i < Total_Registros; i++) {
        printf(" - id=%d nome=%s\n", banco[i].id, banco[i].nome);
    }
}

// Processa a string de requisição e executa a ação correspondente
void *processar(void *arg) {
    char *requisicao = (char *)arg;
    printf("[RECEBIDO] %s\n", requisicao);

    int OPC, ID;
    char Nome[50];
    char resposta[512] = "[ERRO] Requisição inválida.\n";

    if (sscanf(requisicao, "%d", &OPC) != 1) {
        printf("[ERRO] Formato inválido da requisição.\n");
        free(requisicao);
        return NULL;
    }

    switch (OPC) {
        case 1: // Inserir
            if (sscanf(requisicao, "1 id=%d nome='%49[^']'", &ID, Nome) == 2) {
                pthread_mutex_lock(&m_banco);
                Inserir(ID, Nome);
                snprintf(resposta, sizeof(resposta), "[INSERIDO] id=%d nome=%s\n", ID, Nome);
                pthread_mutex_unlock(&m_banco);
            }
            break;
        case 2: // Remover
            if (sscanf(requisicao, "2 id=%d", &ID) == 1) {
                pthread_mutex_lock(&m_banco);
                Remover(ID);
                SalvarTxt();
                snprintf(resposta, sizeof(resposta), "[REMOVIDO] id=%d\n", ID);
                pthread_mutex_unlock(&m_banco);
            }
            break;
        case 3: // Buscar
            pthread_mutex_lock(&m_banco);
            if (sscanf(requisicao, "3 id=%d", &ID) == 1) {
                SelectId(ID);
            } else if (sscanf(requisicao, "3 nome='%49[^']'", Nome) == 1) {
                SelectNome(Nome);
            }
            pthread_mutex_unlock(&m_banco);
            break;
        case 4: // Atualizar
            if (sscanf(requisicao, "4 id=%d nome='%49[^']'", &ID, Nome) == 2) {
                pthread_mutex_lock(&m_banco);
                Atualizar(ID, Nome);
                SalvarTxt();
                snprintf(resposta, sizeof(resposta), "[ATUALIZADO] id=%d nome=%s\n", ID, Nome);
                pthread_mutex_unlock(&m_banco);
            }
            break;
        case 5: // Listar
            pthread_mutex_lock(&m_banco);
            Listar();
            pthread_mutex_unlock(&m_banco);
            break;
        default:
            printf("[ERRO] Código de operação desconhecido: %d\n", OPC);
    }

    // Envia resposta de volta pelo FIFO
    int fd_resp = open("/tmp/resposta", O_WRONLY);
    if (fd_resp != -1) {
        write(fd_resp, resposta, strlen(resposta) + 1);
        close(fd_resp);
    }

    free(requisicao);
    return NULL;
}

// Função principal: inicializa o servidor e as threads
int main() {
    CarregarTxt();

    char *fifo_requisicao = "/tmp/myfifo";
    char *fifo_resposta = "/tmp/resposta";

    mkfifo(fifo_requisicao, 0666);
    mkfifo(fifo_resposta, 0666);

    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    // Cria as threads que vão ficar ouvindo e processando as tarefas
    for (int i = 0; i < THREAD_NUM; i++) {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->id_thread = i;
        pthread_create(&th[i], NULL, Desenfileirar, args);
        printf("[THREAD] Thread %d criada e pronta!\n", i);
    }

    // Laço principal: recebe requisições do FIFO e as enfileira
    char buffer[256];
    while (1) {
        int fd = open(fifo_requisicao, O_RDONLY);
        if (fd == -1) continue;

        while (read(fd, buffer, sizeof(buffer)) > 0) {
            Task t;
            t.requisicao = strdup(buffer);
            Enfileirar(t);
        }

        close(fd);
    }

    return 0;
}
