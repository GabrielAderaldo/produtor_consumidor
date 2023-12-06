#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // Para o sleep

#define MAX_PRODUCED 100
#define BUFFER_SIZE 20
#define NUM_CONSUMERS 4

sem_t full, empty;
pthread_mutex_t mutex;

int buffer[BUFFER_SIZE] = {0};
int produced_count = 0;
int consumed_count = 0;

void *producer(void *arg) {
    while (produced_count < MAX_PRODUCED) {
        // Solicitação de recursos
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        // Verifica se há espaço no buffer para produzir
        int i = 0;
        while (buffer[i] != 0 && i < BUFFER_SIZE) {
            i++;
        }

        if (i < BUFFER_SIZE) {
            buffer[i] = produced_count + 1;
            printf("Produtor produz um item:\nBarra: [ ");
            for (int j = 0; j < BUFFER_SIZE; ++j) {
                if (buffer[j] != 0) {
                    printf("# ");
                } else {
                    printf("  ");
                }
            }
            printf("]\n");

            produced_count++;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }

    printf("Produtor encerrou a produção.\n");
    pthread_exit(NULL);
}


void *consumerComDeadLock(void *arg) {
    int consumer_id = *((int *)arg);

    while (1) {
        // Solicitação de recursos
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        // Tentativa de consumir um item do buffer, independentemente se há itens ou não
        printf("Consumidor_%d tenta consumir um item:\n", consumer_id);

        // Simula o consumo de um item
        // Aqui é onde normalmente o consumo ocorreria se houvesse um item para consumir

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        // Liberando recursos
        sem_post(&full);
        sleep(2); // Adiciona um atraso de 2 segundos para demonstrar as operações
    }

    printf("Consumidor_%d encerrou o consumo.\n", consumer_id);
    pthread_exit(NULL);
}


void *consumer(void *arg) {
    int consumer_id = *((int *)arg);

    while (1) {
        // Solicitação de recursos
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        // Verifica se há itens no buffer para consumir
        int i = 0;
        while (buffer[i] == 0 && i < BUFFER_SIZE) {
            i++;
        }

        if (i < BUFFER_SIZE) {
            printf("Consumidor_%d consome o item %d:\nBarra: [ ", consumer_id, buffer[i]);
            buffer[i] = 0;
            for (int j = 0; j < BUFFER_SIZE; ++j) {
                if (buffer[j] != 0) {
                    printf("# ");
                } else {
                    printf("  ");
                }
            }
            printf("]\n");
            consumed_count++;
        } else {
            if (produced_count >= MAX_PRODUCED) {
                printf("Consumidor_%d: Não há mais produtos para consumir.\n", consumer_id);
                pthread_mutex_unlock(&mutex);
                sem_post(&full);
                break;
            }
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        // Liberando recursos
        sem_post(&full);
        sleep(2); // Adiciona um atraso de 2 segundos para demonstrar as operações
    }

    printf("Consumidor_%d encerrou o consumo.\n", consumer_id);
    pthread_exit(NULL);
}

int main() {
    pthread_t producer_thread, consumer_threads[NUM_CONSUMERS];
    int consumer_ids[NUM_CONSUMERS];

    // Inicialização dos semáforos e mutex
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    pthread_mutex_init(&mutex, NULL);

    // Criação das threads produtor e consumidor
    pthread_create(&producer_thread, NULL, producer, NULL);

    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumer_threads[i], NULL, consumer, &consumer_ids[i]);
    }

    // Aguarda a thread produtora terminar
    pthread_join(producer_thread, NULL);

    // Aguarda as threads consumidoras terminarem
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        pthread_join(consumer_threads[i], NULL);
    }

    // Destroi semáforos e mutex
    sem_destroy(&full);
    sem_destroy(&empty);
    pthread_mutex_destroy(&mutex);

    return 0;
}
