//
// Generated using bf2c: https://github.com/ikbenlike/bf2c
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
int main(void){
    puts("I'm prefixed!");
    char *array = calloc(30000, sizeof(char));
    int dptr = 0;
    if(array == NULL){
        fprintf(stderr, "%s: %s\n", __FILE__, strerror(errno));
        exit(1);
    }
