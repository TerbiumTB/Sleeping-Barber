#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#ifndef SALON_READ_WRITE_H
#define SALON_READ_WRITE_H


struct {
    FILE *input;
    FILE *output;
    bool console;
    bool is_init;

} rw;

int rw_init(char *, char *, bool);

void rw_read(int *, int *);

void rw_generate(int *, int, int);

void rw_write(char *, const int *);

void rw_close();

#endif //SALON_READ_WRITE_H
