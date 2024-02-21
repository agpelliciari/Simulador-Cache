#include "lib.h"

bool validarArgumentos(int argc, char *argv[]){

    int cantidadArgumentos = argc;

    if (!(cantidadArgumentosEsValida(cantidadArgumentos))){
        /* Error cantidad de argumentos invalidos */
        fprintf(stderr, "Error: cantidad de argumentos invalida.\n");
        return false;
    }

    if (!(existeArchivo(argv))){
        /* Error archivo no existe */
        fprintf(stderr, "Error: no existe el archivo.\n");
        return false;
    }

    if (!esPotenciaDeDos(atoi(argv[TAMANIO_CACHE])) || !esPotenciaDeDos(atoi(argv[ASOCIATIVIDAD_CACHE])) || !esPotenciaDeDos(atoi(argv[SETS_CACHE]))){
        /* Error argumentos no son potencia de dos */
        fprintf(stderr, "Error: argumento no es potencia de dos.\n");
        return false;
    }
    
    if (!(combinacionDeArgumentosEsValida(argv))){
        /* Error combinacion de argumentos es invalidad */
        fprintf(stderr, "Error: combinacion de argumentos invalida.\n");
        return false;
    }
    
    if (!(modoVerboso(argc, argv))){
        /* Error en el modo verboso */
        fprintf(stderr, "Error: Los argumentos del modo verboso no son numeros que cumplan 0 ≤ n ≤ m.\n");
        return false;
    }

    return true;  
}

bool esPotenciaDeDos(int arg){
    if (arg <= 0) return false;
    return (arg & (arg - 1)) == 0;
}

bool cantidadArgumentosEsValida(int argc){
    return (argc-1) != 4 || (argc-1) != 7;
}

bool existeArchivo(char *argv[]){
    char *arg = argv[1];

    FILE *archivo = fopen(arg, "r");
    if (!(archivo)) return false;
    fclose(archivo);
    return true;
}

bool combinacionDeArgumentosEsValida(char *argv[]){
    int tamañoCache = atoi(argv[2]);
    int asociatividadCache = atoi(argv[3]);
    int setsCache = atoi(argv[4]);

    if (tamañoCache < (setsCache * asociatividadCache)) return false;
    return true;
}

bool modoVerboso(int argc, char *argv[]){
    if (argc == ARGUMENTOS_MAX){
        int n = atoi(argv[INICIO_VERBOSO]);
        int m = atoi(argv[FIN_VERBOSO]);
        if (n < 0 || m < 0 || n > m){
            return false;
        }
    }
    return true;
}

int calcularLog2(int num){
    return (int)log2(num);
}

unsigned int generarMascaraSet(int n, int offset){
    return (((1 << n) - 1) << offset);
}

unsigned int generarMascaraTag(int n){
    unsigned int mask = 0xFFFFFFFF;
    return (mask << n);
}
