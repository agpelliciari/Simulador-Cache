#include "lib.h"
#include "cache.h"

int main (int argc, char *argv[]){  
 
    if (!(validarArgumentos(argc, argv))) return 1;
    
    char *arg = argv[ARCHIVO_TRAZA];
    int tamañoCache = atoi(argv[TAMANIO_CACHE]);
    int asociatividadCache = atoi(argv[ASOCIATIVIDAD_CACHE]);
    int setsCache = atoi(argv[SETS_CACHE]);
 
    cache_t* cache = armarCache(tamañoCache, setsCache, asociatividadCache);

    verboso_t* verboso;
    if (argc == ARGUMENTOS_MAX)
    {
        verboso = inciarModoVerboso(atoi(argv[INICIO_VERBOSO]), atoi(argv[FIN_VERBOSO]));
    }
    else
    {
        verboso = NULL;
    }
    
    FILE *archivo = fopen(arg, "r");
    procesarLineas(archivo, cache, verboso);
    fclose(archivo);

    imprimirEstadisticas(cache);
    
    liberarVerboso(verboso);
    liberarCache(cache);

    return 0;
}
