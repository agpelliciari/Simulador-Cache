#ifndef _LIB_HH_
#define _LIB_HH_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdint.h>

#define ESCRITURA "W"
#define LECTURA "R"

#define TAMANIO_PALABRA 32
#define TAMANIO_MAX 100

#define OPERACION 2
#define CANTIDAD_BYTES 4

#define PENALTY 100
#define LINEA_VACIA 0x00

#define TIPO_OPERACION 0
#define DIRECCION_ACCEDIDA 1
#define BYTES 2

#define ARGUMENTOS_MAX 8
#define ARCHIVO_TRAZA 1
#define TAMANIO_CACHE 2
#define ASOCIATIVIDAD_CACHE 3
#define SETS_CACHE 4
#define INICIO_VERBOSO 6
#define FIN_VERBOSO 7

#define HIT "1"
#define CLEAN_MISS "2a"
#define DIRTY_MISS "2b"

// VALIDACION DE ARGUMENTOS
bool validarArgumentos(int argc, char *argv[]);

bool esPotenciaDeDos(int arg);

bool cantidadArgumentosEsValida(int argc);

bool existeArchivo(char *argv[]);

bool combinacionDeArgumentosEsValida(char *argv[]);

bool modoVerboso(int argc, char *argv[]);

//AUXILIARES
int calcularLog2(int num);

unsigned int generarMascaraSet(int n, int offset);

unsigned int generarMascaraTag(int n);

#endif