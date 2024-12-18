#include "read-write.h"

int rw_init(char *input, char *output, bool console) {
    if (input != NULL) {
        rw.input = fopen(input, "r");
        if (rw.input == NULL) {
            puts("Error: couldn't open input file");
            fclose(rw.input);
            return -1;
        }
    }
    if (output != NULL) {
        rw.output = fopen(output, "w");
        if (rw.output == NULL) {
            puts("Error: couldn't open output file");
            fclose(rw.output);
            return -1;
        }
    }
    rw.console = console;
    rw.is_init = true;
    return 0;
}

//Считываем входные данные в соответствии с аргументами командной строки
void rw_read(int *m, int *n) {
    if (rw.input != NULL) {
        fscanf(rw.input, "%i %i", m, n);

    } else {
        if (*m == -1) {
            printf("Введите количество посетителей за день: ");
            scanf("%i", m);
        }
        if (*n == -1) {
            printf("Введите количество стульев для ожидания в салоне: ");
            scanf("%i", n);
        }

    }
}

//Генерируем посетителей
void rw_generate(int *id, int size, int seed) {
    if (rw.input != NULL) {
        for (int i = 0; i < size; ++i) {
            fscanf(rw.input, "%i", id + i);
        }
    } else {
        srand(seed);
        for (int i = 0; i < size; ++i) {
            id[i] = rand();
        }
    }
}

//Создаем сообщение, возможно вставляя id клиента согласно формату
void rw_write(char *fmt, const int *id) {
    char msg[250];
    if (id != NULL) {
        sprintf(msg, fmt, *id);
    } else {
        strcpy(msg, fmt);
    }
    if (rw.output != NULL) {
        fputs(msg, rw.output);
        fputc('\n', rw.output);
    }
    if (rw.console) {
        puts(msg);
    }
}

void rw_close() {
    fclose(rw.input);
    fclose(rw.output);
    rw.is_init = false;
}