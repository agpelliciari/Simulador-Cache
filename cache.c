#include "cache.h"

cache_t *armarCache(int tamanio, int cantidadSets, int lineasPorSet){
    cache_t* cache = malloc(sizeof(cache_t));
    cache->tamanio = tamanio;
    cache->cantidadSets = cantidadSets;
    cache->lineasPorSet = lineasPorSet;
    cache->bytesPorBloque = tamanio / (cantidadSets*lineasPorSet);

    cache->bitsByteOffset = calcularLog2(cache->bytesPorBloque);      //Obtengo cant bits para byte offset
    cache->bitsSetOffset = calcularLog2(cache->cantidadSets);         //Obtengo cant bits para set offset

    array_t* sets = crearArray(cantidadSets, lineasPorSet);
    cache->sets = sets;

    stats_t* estadisticasCache = crearEstadisticas();
    cache->estadisticas = estadisticasCache;

    return cache;
}

void liberarCache(cache_t *cache){
    array_t* sets = cache->sets;
    liberarArray(sets, cache);

    stats_t* estadisticas = cache->estadisticas;
    liberarEstadisticas(estadisticas);
    
    if (cache != NULL)
    {
        free(cache);
    }
}

array_t *crearArray(int cantidad, int capacidad){
    array_t* nuevoArray = malloc(cantidad * sizeof(array_t));
    for (int i = 0; i < cantidad; i++)
    {
        nuevoArray[i].cantidad = 0;
        nuevoArray[i].tamanio = capacidad;
        nuevoArray[i].datos = malloc(capacidad * sizeof(via_t));

        for (int j = 0; j < capacidad; j++)
        {
            nuevoArray[i].datos[j].dirtyBit = false;
            nuevoArray[i].datos[j].numTransaccion = 0;
            nuevoArray[i].datos[j].tag = 0x00;
        }
    }
    return nuevoArray;
}

void liberarArray(array_t *sets, cache_t* cache){
    if (sets != NULL)
    {
        for (int i = 0; i < cache->cantidadSets; i++)
        {
            free(sets[i].datos);
        }
        free(sets);
    }
}

stats_t *crearEstadisticas(){
    stats_t* estadisticas = malloc(sizeof(stats_t));
    estadisticas->escrituras = 0;
    estadisticas->missesEscritura = 0;
    estadisticas->cantBytesEscritos = 0;
    estadisticas->dirtyMissesEscritura = 0;
    estadisticas->tiempoAccesoEscritura = 0;
    estadisticas->lecturas = 0;
    estadisticas->missesLectura = 0;
    estadisticas->dirtyMissesLectura = 0;
    estadisticas->tiempoAccesoLectura = 0;
    estadisticas->missRateTotal = 0;
    estadisticas->totalAccesos = 0;
    estadisticas->totalMisses = 0;
    estadisticas->cantBytesLeidos = 0;
    
    return estadisticas;
}

void calcularEstadisticas(char* array[], cache_t* cache, bool *esHit, bool *esDirty){
    if (strcmp(array[TIPO_OPERACION], LECTURA) == 0)
    {
        cache->estadisticas->lecturas += 1;

        if (*esHit)
        {
            cache->estadisticas->tiempoAccesoLectura += 1;
        }
        else
        {
            cache->estadisticas->missesLectura += 1;
            cache->estadisticas->totalMisses += 1;
            cache->estadisticas->cantBytesLeidos += cache->bytesPorBloque;
            if (*esDirty)
            {
                cache->estadisticas->dirtyMissesLectura += 1;
                cache->estadisticas->tiempoAccesoLectura += (1 + (2*PENALTY));
                cache->estadisticas->cantBytesEscritos += cache->bytesPorBloque;
            }
            else
            {
                cache->estadisticas->tiempoAccesoLectura += (1 + PENALTY);
            }
        }
    }
    else 
    {
        cache->estadisticas->escrituras += 1;
        
        if (*esHit)
        {
            cache->estadisticas->tiempoAccesoEscritura += 1;
        }
        else
        {
            cache->estadisticas->missesEscritura += 1;
            cache->estadisticas->totalMisses += 1;
            cache->estadisticas->cantBytesLeidos += cache->bytesPorBloque;
            if (*esDirty)
            {
                cache->estadisticas->dirtyMissesEscritura += 1;
                cache->estadisticas->tiempoAccesoEscritura += (1 + (2*PENALTY));
                cache->estadisticas->cantBytesEscritos += cache->bytesPorBloque;
            }
            else
            {
                cache->estadisticas->tiempoAccesoEscritura += (1 + PENALTY);
            }
        }
    }
    cache->estadisticas->totalAccesos += 1;
}

void imprimirEstadisticas(cache_t *cache){
    if (cache->lineasPorSet == 1)
    {
        fprintf(stdout, "direct-mapped, %d sets, size = %dKB\n", cache->cantidadSets, (cache->tamanio)/1000);
    }
    else
    {
        fprintf(stdout, "%d-way, %d sets, size = %dKB\n", cache->lineasPorSet, cache->cantidadSets, (cache->tamanio)/1000);
    }
    fprintf(stdout, "loads %d stores %d total %d\n", cache->estadisticas->lecturas, cache->estadisticas->escrituras, cache->estadisticas->totalAccesos);
    fprintf(stdout, "rmiss %d wmiss %d total %d\n", cache->estadisticas->missesLectura, cache->estadisticas->missesEscritura, cache->estadisticas->totalMisses);
    fprintf(stdout, "dirty rmiss %d dirty wmiss %d\n", cache->estadisticas->dirtyMissesLectura, cache->estadisticas->dirtyMissesEscritura);
    fprintf(stdout, "bytes read %d bytes written %d\n", cache->estadisticas->cantBytesLeidos, cache->estadisticas->cantBytesEscritos);
    fprintf(stdout, "read time %d write time %d\n", cache->estadisticas->tiempoAccesoLectura, cache->estadisticas->tiempoAccesoEscritura);
    cache->estadisticas->missRateTotal += ((double)(cache->estadisticas->totalMisses) / (double)(cache->estadisticas->totalAccesos));
    fprintf(stdout, "miss rate %f\n", cache->estadisticas->missRateTotal);
}

void liberarEstadisticas(stats_t *estadisticas){
    if (estadisticas != NULL)
    {
        free(estadisticas);
    }
}

void procesarLineas(FILE *archivo, cache_t* cache, verboso_t* verboso){
    char linea[TAMANIO_MAX];
    char *array[TAMANIO_MAX];

    cache->sets->transacciones = 0;

    estadisticas_verboso_t* estadisticasV = malloc(sizeof(estadisticas_verboso_t));
    
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        int contador = 0;
        int indice = 0;
        char *token;
        
        token = strtok(linea, " :");

        while(token != NULL) {
            contador++;
            if (contador >= OPERACION && contador <= CANTIDAD_BYTES) {
                array[indice] = token;
                indice++;
            }
            token = strtok(NULL, " :");
        }
        ingresarDato(array, cache, verboso, estadisticasV);
    }

    free(estadisticasV);
}

void ingresarDato(char* array[], cache_t* cache, verboso_t* verboso, estadisticas_verboso_t* estadisticasV){
    array_t* sets = cache->sets;

    limpiarEstadisticas(estadisticasV, sets->transacciones);

    unsigned int direccMemoriaAccedida = strtoul(array[DIRECCION_ACCEDIDA], NULL, 16);
   
    unsigned int set = obtenerSet(cache, direccMemoriaAccedida);
    //estadisticasV->set = set;      //VERBOSO

    unsigned int tag = obtenerTag(cache, direccMemoriaAccedida);
    //estadisticasV->tag = tag;      //VERBOSO

    bool esHit = false;
    bool esDirty = false;
    
    analizarCaso(sets, estadisticasV, array, set, tag, &esHit, &esDirty);

    if (verboso != NULL && modoVerbosoActivo(verboso, cache))
    {
        imprimirLineaVerboso(estadisticasV, cache);
    }
    
    calcularEstadisticas(array, cache, &esHit, &esDirty);

    sets->transacciones++;
}

void analizarCaso(array_t *setsCache, estadisticas_verboso_t* estadisticasV, char* array[], unsigned int setAccedido, unsigned int tagAccedido, bool* esHit, bool* esDirty){
    int i = 0;

    estadisticasV->set = setAccedido;      //VERBOSO

    estadisticasV->tag = tagAccedido;      //VERBOSO

    while (noHit(*esHit, i, setsCache[setAccedido].cantidad))
    {
        if (hit(setsCache[setAccedido].datos[i].tag, tagAccedido))           // Logica para hit
        {
            estadisticasV->lastUsed = setsCache[setAccedido].datos[i].numTransaccion;            //VERBOSO

            *esHit = true;
            setsCache[setAccedido].datos[i].numTransaccion = setsCache->transacciones;
            
            estadisticasV->caso = HIT;             //VERBOSO
            estadisticasV->numerolinea = i;        //VERBOSO
            estadisticasV->prevTag = tagAccedido;  //VERBOSO
            estadisticasV->validBit = 1;           //VERBOSO

            if (esDirtyBit(setsCache[setAccedido].datos[i].dirtyBit))
            {
                estadisticasV->dirtyBit = 1;        //VERBOSO
            }
            
            if (strcmp(array[TIPO_OPERACION], ESCRITURA) == 0)
            {
                setsCache[setAccedido].datos[i].dirtyBit = true;
            }
        }
        
        i++;
    }

    if (!(*esHit))                                                           // Logica para miss
    {
        i = 0;
        bool posicion_encontrada = false;

        if (hayLineaVacia(setsCache[setAccedido].cantidad, setsCache[setAccedido].tamanio))              // Existe una linea que se encuentra vacia
        {
            while (!posicion_encontrada && i <= setsCache[setAccedido].cantidad)
            {
                if (setsCache[setAccedido].datos[i].tag == LINEA_VACIA)
                {
                    posicion_encontrada = true;
                    setsCache[setAccedido].datos[i].tag = tagAccedido;
                    setsCache[setAccedido].datos[i].numTransaccion = setsCache->transacciones;
                    setsCache[setAccedido].cantidad++;
                    
                    estadisticasV->caso = CLEAN_MISS; //VERBOSO
                    estadisticasV->numerolinea = i;   //VERBOSO
                    estadisticasV->prevTag = - 1;     //VERBOSO
                   
                    if (strcmp(array[TIPO_OPERACION], ESCRITURA) == 0)
                    {
                        setsCache[setAccedido].datos[i].dirtyBit = true;
                    }
                }
            
                i++;
            }
        }
        else                                                          // Se debe sustituir una linea: menor numero de transaccion
        {
            int minimo = 0;
            i = 1;
            while (i < setsCache[setAccedido].cantidad)
            {
                if (setsCache[setAccedido].datos[i].numTransaccion < setsCache[setAccedido].datos[minimo].numTransaccion)
                {
                    minimo = i;
                }
                else
                {
                    i++;
                }
            }

            if (esDirtyBit(setsCache[setAccedido].datos[minimo].dirtyBit))
            {
                setsCache[setAccedido].datos[minimo].dirtyBit = false;
                *esDirty = true;

                estadisticasV->caso = DIRTY_MISS;                    //VERBOSO
                estadisticasV->dirtyBit = 1;                         //VERBOSO
            }
            else
            {
                estadisticasV->caso = CLEAN_MISS;                    //VERBOSO
            }
            
            estadisticasV->lastUsed = setsCache[setAccedido].datos[minimo].numTransaccion;      //VERBOSO
            estadisticasV->prevTag = setsCache[setAccedido].datos[minimo].tag;    //VERBOSO
            estadisticasV->numerolinea = minimo;                     //VERBOSO
            estadisticasV->validBit = 1;                             //VERBOSO

            setsCache[setAccedido].datos[minimo].tag = tagAccedido;
            setsCache[setAccedido].datos[minimo].numTransaccion = setsCache->transacciones;

            if (strcmp(array[TIPO_OPERACION], ESCRITURA) == 0)
            {
                setsCache[setAccedido].datos[minimo].dirtyBit = true;
            }
        }
    }
}

verboso_t* inciarModoVerboso(int ini, int fin){
    verboso_t* verboso = malloc(sizeof(verboso_t));
    verboso->activo = false;
    verboso->inicio = ini;
    verboso->final = fin;
    return verboso;
}

bool modoVerbosoActivo(verboso_t* verboso, cache_t* cache){
    if (verboso->activo)
    {
        if (verboso->final == cache->sets->transacciones)
        {
            verboso->activo = false;
        }

        return true;
    }

    if (verboso->inicio == cache->sets->transacciones)
    {
        verboso->activo = true;
        return true;
    }

    return false;
    
    
}

void limpiarEstadisticas(estadisticas_verboso_t* estadisticasV, int indice){
    estadisticasV->indice = indice;
    estadisticasV->caso = " ";
    estadisticasV->set = 0;
    estadisticasV->tag = 0;
    estadisticasV->numerolinea = 0;
    estadisticasV->prevTag = 0;
    estadisticasV->validBit = 0;
    estadisticasV->dirtyBit = 0;
    estadisticasV->lastUsed = 0;
}

void imprimirLineaVerboso(estadisticas_verboso_t* estadisticasV, cache_t* cache){
    fprintf(stdout, "%i %s %x %x %i ", estadisticasV->indice, estadisticasV->caso, estadisticasV->set, estadisticasV->tag, estadisticasV->numerolinea);
    
    if (estadisticasV->prevTag != -1)
    {
        fprintf(stdout, "%x ", estadisticasV->prevTag);
    }
    else
    {
        fprintf(stdout, "%i ", estadisticasV->prevTag);
    }

    fprintf(stdout, "%i %i", estadisticasV->validBit, estadisticasV->dirtyBit);

    if (cache->lineasPorSet > 1)
    {
        fprintf(stdout, " %i", estadisticasV->lastUsed);
    }

    fprintf(stdout, "\n");
}

void liberarVerboso(verboso_t *verboso){
    if (verboso != NULL)
    {
        free(verboso);
    }
}

unsigned int obtenerSet(cache_t *cache, unsigned int direccMemoriaAccedida){
    unsigned int setMask = generarMascaraSet(cache->bitsSetOffset, cache->bitsByteOffset);

    unsigned int set = (direccMemoriaAccedida & setMask) >> cache->bitsByteOffset;

    return set;
}

unsigned int obtenerTag(cache_t* cache, unsigned int direccMemoriaAccedida){
    unsigned int tagMask = generarMascaraTag(cache->bitsSetOffset + cache->bitsByteOffset);

    unsigned int tag = (direccMemoriaAccedida & tagMask) >> (cache->bitsByteOffset + cache->bitsSetOffset);

    return tag;
}

bool noHit(bool esHit, int indiceLinea, int cantidadLineas){
    return !esHit && indiceLinea < cantidadLineas;
}

bool hit(unsigned int tagLinea, unsigned int tagBuscado){
    return tagLinea == tagBuscado;
}

bool hayLineaVacia(int cantidadActual, int cantidadMaxima){
    return cantidadActual != cantidadMaxima;
}

bool esDirtyBit(bool dirty){
    return dirty;
}