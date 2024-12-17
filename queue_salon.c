#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
/*#include <stdbool.h>
 * кажется, начиная с 23 стандарта не нужен, но если вдруг ругается, надо раскоментировать*/

#include "read-write.h"

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
    Salon *salon = (Salon *) param;

    while (1) {
        //барбер спит пока нет клиентов
        pthread_mutex_lock(salon->mutex);

        //если барбер обсулжил всех посетителей за день, салон закрывается
        if (salon->all_visited) {
            rw_write("Все желающие уже посетили салон", NULL);
            break;
        }
        while (salon->queue->waiting == 0) {
            pthread_mutex_unlock(salon->mutex);
            sleep(1);
            pthread_mutex_lock(salon->mutex);
        }

        rw_write("Барбер проснулся и начинает стричь всех в порядке очереди", NULL);

        pthread_mutex_unlock(salon->mutex);


        //барбер начинает стричь всех ожидающих в порядке очереди
        pthread_mutex_lock(salon->mutex);
        int top;
        while (salon->queue->waiting) {
            top = salon->queue->served;
            rw_write("Барбер начал стрижку %i клиента", &salon->queue->customers[top]);

            pthread_mutex_unlock(salon->mutex);

            sleep(2);

            pthread_mutex_lock(salon->mutex);
            rw_write("Барбер закончил стрижку %i клиента", &salon->queue->customers[top]);

            ++(salon->queue->served);
            --(salon->queue->waiting);

        }

        rw_write("Барбер обслужил всех кто был в очереди и пошел спать", NULL);

        //очередь закончилась и барбер пошел спать
        pthread_mutex_unlock(salon->mutex);

    }

    //в салоне побывало максимальное количество посетителей и он закрывается
    rw_write("Салон закрывается", NULL);

    return NULL;
}

void *customer_thread(void *param) {
    Customer customer = *(Customer *) param;

    //посетитель ждет момента войти в салон
    pthread_mutex_lock(customer.mutex);
    rw_write("Посетитель %i вошел в салон", &customer.id);


    //если мест нет, он тут же выходит
    if (customer.queue->waiting >= customer.queue->capacity) {
        rw_write("Посетителю %i не хватило места и он ушел", &customer.id);

        pthread_mutex_unlock(customer.mutex);
        return NULL;
    }


    //иначе он занимет очередь и ждет стрижки
    rw_write("Посетитель %i ждет свою очередь", &customer.id);

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
    rw_write("Посетитель %i был обсулжен и покинул салон", &customer.id);
    pthread_mutex_unlock(customer.mutex);
    return NULL;
}


int main(int argc, char *argv[]) {
    //выводить ли данные на консоль
    bool console = true;

    //путь к входному файлу
    char *input = NULL;

    //путь к выходному файлу
    char *output = NULL;

    //seed для генерации рандомных чисел
    int seed = 13;

    //количество стульев в салоне
    int capacity = -1;

    //количество посетителей за день
    int max_visitors = -1;

    char f;
    for (int i = 1; i < argc; ++i) {
        f = argv[i][1];
        ++i;
        switch (f) {
            case 'm':
                max_visitors = atoi(argv[i]);
                break;
            case 'n':
                capacity = atoi(argv[i]);
                break;
            case 's':
                seed = atoi(argv[i]);
                break;
            case 'i':
                input = argv[i];
                break;
            case 'o':
                output = argv[i];
                break;
            case 'c':
                console = (argv[i][0] == '1');
                break;
            default:
                printf("Error: incorrect flag or format");
                return 0;
        }
    }
    int visitors_id[max_visitors];
    int err;

    //инициализируем чтение запись в соответсвии с атрибутами консоли
    if (err = rw_init(input, output, console), err == -1) {
        return 0;
    }

    rw_read(&max_visitors, &capacity, visitors_id, seed);

    //наш основной мьютекс
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

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
            .all_visited = false,
    };


    //создаем поток барбера
    pthread_t barber;
    pthread_create(&barber, NULL, barber_thread, &salon);

    //рассматриваем main как управляющий поток, который порождает посетителей
    Customer visitors[max_visitors];
    pthread_t visitors_thread[max_visitors];
    for (int i = 0; i < max_visitors; ++i) {
        visitors[i].id = visitors_id[i];
        visitors[i].queue = &queue;
        visitors[i].mutex = &mutex;

        pthread_create(visitors_thread + i, NULL, customer_thread, visitors + i);
        sleep(1);
    }

    //сообщаем что все посетители за день пришли
    pthread_mutex_lock(&mutex);
    rw_write("Все желающие посетили салон", NULL);
    salon.all_visited = true;
    pthread_mutex_unlock(&mutex);


    //ждем пока барбер закончит работать
    pthread_join(barber, NULL);
    rw_close();
    free(queue.customers);
    return 0;
}
