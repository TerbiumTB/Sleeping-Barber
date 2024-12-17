#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
/*#include <stdbool.h>
 * кажется, начиная с 23 стандарта не нужен, но если вдруг ругается, надо расскоментировать*/

typedef struct CustomerQueue_ {
    int *customers;
    int served;
    int waiting;
    int capacity;
} CustomerQueue;

typedef struct Customer_ {
    CustomerQueue *queue;
    pthread_mutex_t *mutex;
    int id;
} Customer;

typedef struct Salon_ {
    CustomerQueue *queue;
    pthread_mutex_t *mutex;
    bool all_visited;
} Salon;


void *barber_thread(void *param) {
    //не разыменовываем, так как состояние салона могут менять другие потоки (main)
    Salon * salon = (Salon *) param;

    while (1) {

        //если барбер обсулжил всех посетителей за день, салон закрывается
        pthread_mutex_lock(salon->mutex);
        if (salon->all_visited) {
            break;
        }
        pthread_mutex_unlock(salon->mutex);


        //барбер спит пока нет клиентов
        pthread_mutex_lock(salon->mutex);
        while (salon->queue->waiting == 0) {
            pthread_mutex_unlock(salon->mutex);
            sleep(1);
            pthread_mutex_lock(salon->mutex);
        }

        printf("Барбер проснулся и начинает стричь всех в порядке очереди\n");
        pthread_mutex_unlock(salon->mutex);


        //барбер начинает стричь всех ожидающих в порядке очереди
        pthread_mutex_lock(salon->mutex);
        int top;
        while (salon->queue->waiting) {
            top = salon->queue->served;
            printf("Барбер начал стрижку %i клиента\n", salon->queue->customers[top]);
            pthread_mutex_unlock(salon->mutex);

            sleep(2);

            pthread_mutex_lock(salon->mutex);
            printf("Барбер закончил стрижку %i клиента\n", salon->queue->customers[top]);
            ++(salon->queue->served);
            --(salon->queue->waiting);
        }

        printf("Барбер обслужил всех кто был в очереди и пошел спать\n");
        //очередь закончилась и барбер пошел спать
        pthread_mutex_unlock(salon->mutex);

    }

    //в салоне побывало максимальное количество посетителей и он закрывается
    printf("Салон закрывается\n");
    return NULL;
}

void *customer_thread(void *param) {
    Customer customer = *(Customer *) param;

    //посетитель ждет момента войти в салон
    pthread_mutex_lock(customer.mutex);
    printf("Посетитель %i вошел в салон\n", customer.id);

    //если мест нет, он тут же выходит
    if (customer.queue->waiting >= customer.queue->capacity) {
        printf("Посетителю %i не хватило места и он ушел\n", customer.id);
        pthread_mutex_unlock(customer.mutex);
        return NULL;
    }


    //иначе он занимет очередь и ждет стрижки
    printf("Посетитель %i ждет свою очередь\n", customer.id);
    int number = customer.queue->served + customer.queue->waiting;
    customer.queue->customers[number] = customer.id;
    ++(customer.queue->waiting);
    pthread_mutex_unlock(customer.mutex);


    //посетитель ждет пока его обслужат
    pthread_mutex_lock(customer.mutex);
    while (customer.queue->served <= number) {
        pthread_mutex_unlock(customer.mutex);
        sleep(1);
        pthread_mutex_lock(customer.mutex);
    }

    //посетитель уходит из салона
    printf("Посетитель %i был обсулжен и покинул салон\n", customer.id);
    pthread_mutex_unlock(customer.mutex);
    return NULL;
}


int main() {
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);


    //количество стульев в салоне
    int capacity = 3;

    //количество посетителей за день
    int max_visitors = 10;


    //инициализируем структуру для обработки очереди посетителей
    CustomerQueue queue = {
            .capacity = capacity,
            .waiting = 0,
            .served = 0
    };
    queue.customers = malloc(4 * max_visitors);

    //храним состояние салона
    Salon salon = {
            .mutex = &mutex,
            .queue = &queue,
            .all_visited = false
    };


    //создаем поток барбера
    pthread_t barber;
    pthread_create(&barber, NULL, barber_thread, &salon);

    //рассматриваем main как управляющий поток, который порождает посетителей
    Customer visitors[max_visitors];
    pthread_t visitors_thread[max_visitors];
    for (int i = 0; i < max_visitors; ++i) {
        visitors[i].id = i + 1;
        visitors[i].queue = &queue;
        visitors[i].mutex = &mutex;
        pthread_create(visitors_thread + i, NULL, customer_thread, visitors + i);
        sleep(1);
    }

    //сообщаем что все посетители за день пришли
    pthread_mutex_lock(&mutex);
    printf("Все посетили салон\n");
    salon.all_visited = true;
    pthread_mutex_unlock(&mutex);


    //ждем пока барбер закончит работать
    pthread_join(barber, NULL);

    free(queue.customers);
    return 0;
}
