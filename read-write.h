//
// Created by Тимофей Булгаков on 17.12.2024.
//
#include <stdio.h>
#include <stdlib.h>

#ifndef SALON_READ_WRITE_H
#define SALON_READ_WRITE_H


struct {
    FILE * input;
    FILE * output;
    bool console;
    bool is_init;

} rw;

int rw_init(char *, char *, bool);
void rw_read(int *, int *, int *, int);
void rw_write(char *, int *);
void rw_close();
#endif //SALON_READ_WRITE_H
