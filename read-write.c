#include "read-write.h"


int rw_init(char *input, char *output, bool console) {
    if (input != NULL) {
        rw.input = fopen(input, "r");
        if (rw.input == NULL) {
            printf("Error: couldn't open input file");
            fclose(rw.input);
            return -1;
        }
    }
    if (output != NULL) {
        rw.output = fopen(output, "w");
        if (rw.output == NULL) {
            printf("Error: couldn't open output file");
            fclose(rw.output);
            return -1;
        }
    }
    rw.console = console;
    rw.is_init = true;
    return 0;
}

void rw_read(int *m, int *n, int *id, int seed) {
    //считываем входные данные в соответсвии с аргументами командной строки
    if (rw.input != NULL) {
        fscanf(rw.input, "%i %i", m, n);
        for (int i = 0; i < *m; ++i) {
            fscanf(rw.input, "%i", id + i);
        }
    } else {
        if (*m == -1) {
            scanf("%i", m);
        }
        if (*n == -1) {
            scanf("%i", n);
        }
        srand(seed);
        for (int i = 0; i < *m; ++i) {
            id[i] = rand();
        }
    }

}

void rw_write(char *fmt, int *id) {
    //Создаем сообщение, возможно вставляя id клиента согласно формату
    char msg[250];
    sprintf(msg, fmt, id);
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