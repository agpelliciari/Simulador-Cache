#include "cache.h"

cache_t *armarCache(int tamanio, int cantidadSets, int lineasPorSet){
    cache_t* cache = malloc(sizeof(cache_t));
    cache->tamanio = tamanio;
    cache->cantidadSets = cantidadSets;
    cache->lineasPorSet = lineasPorSet;
    int bytesPorBloque = tamanio / (cantidadSets*lineasPorSet);
    cache->bytesPorBloque = bytesPorBloque;

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

void calcularEstadisticas(char* array[], cache_t* cache, bool esHit, bool esDirty){
    if (strcmp(array[TIPO_OPERACION], LECTURA) == 0)
    {
        cache->estadisticas->lecturas += 1;

        if (esHit)
        {
            cache->estadisticas->tiempoAccesoLectura += 1;
        }
        else
        {
            cache->estadisticas->missesLectura += 1;
            cache->estadisticas->totalMisses += 1;
            cache->estadisticas->cantBytesLeidos += cache->bytesPorBloque;
            if (esDirty)
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
        
        if (esHit)
        {
            cache->estadisticas->tiempoAccesoEscritura += 1;
        }
        else
        {
            cache->estadisticas->missesEscritura += 1;
            cache->estadisticas->totalMisses += 1;
            cache->estadisticas->cantBytesLeidos += cache->bytesPorBloque;
            if (esDirty)
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

    int bitsByteOffset = calcularLog2(cache->bytesPorBloque);                          //Obtengo cant bits para byte offset
    int bitsSetOffset = calcularLog2(cache->cantidadSets);                             //Obtengo cant bits para set offset

    unsigned int setMask = generarMascaraSet(bitsSetOffset, bitsByteOffset);
    unsigned int tagMask = generarMascaraTag(bitsSetOffset + bitsByteOffset);

    unsigned int set = (direccMemoriaAccedida & setMask) >> bitsByteOffset;
    estadisticasV->set = set;      //VERBOSO
    unsigned int tag = (direccMemoriaAccedida & tagMask) >> (bitsByteOffset + bitsSetOffset);
    estadisticasV->tag = tag;      //VERBOSO

    bool esHit = false;
    bool esDirty = false;
    int i = 0;

    while (!esHit && i < sets[set].cantidad)
    {
        if (sets[set].datos[i].tag == tag)           // Logica para hit
        {
            estadisticasV->lastUsed = sets[set].datos[i].numTransaccion;            //VERBOSO

            esHit = true;
            sets[set].datos[i].numTransaccion = sets->transacciones;
            
            estadisticasV->caso = HIT;             //VERBOSO
            estadisticasV->numerolinea = i;        //VERBOSO
            estadisticasV->prevTag = tag;          //VERBOSO
            estadisticasV->validBit = 1;           //VERBOSO

            if (sets[set].datos[i].dirtyBit)
            {
                estadisticasV->dirtyBit = 1;        //VERBOSO
            }
            
            if (strcmp(array[TIPO_OPERACION], ESCRITURA) == 0)
            {
                sets[set].datos[i].dirtyBit = true;
            }
        }
        
        i++;
    }

    if (!esHit)                                       // Logica para miss
    {
        i = 0;
        bool posicion_encontrada = false;

        if (sets[set].cantidad != sets[set].tamanio)                  // Existe una linea que se encuentra vacia
        {
            while (!posicion_encontrada && i <= sets[set].cantidad)
            {
                if (sets[set].datos[i].tag == LINEA_VACIA)
                {
                    posicion_encontrada = true;
                    sets[set].datos[i].tag = tag;
                    sets[set].datos[i].numTransaccion = sets->transacciones;
                    sets[set].cantidad++;
                    
                    estadisticasV->caso = CLEAN_MISS; //VERBOSO
                    estadisticasV->numerolinea = i;   //VERBOSO
                    estadisticasV->prevTag = - 1;     //VERBOSO
                   
                    if (strcmp(array[TIPO_OPERACION], ESCRITURA) == 0)
                    {
                        sets[set].datos[i].dirtyBit = true;
                    }
                }
            
                i++;
            }
        }
        else                                                          // Se debe sustituir una linea: menor numero de transaccion
        {
            int minimo = 0;
            i = 1;
            while (i < sets[set].cantidad)
            {
                if (sets[set].datos[i].numTransaccion < sets[set].datos[minimo].numTransaccion)
                {
                    minimo = i;
                }
                else
                {
                    i++;
                }
            }

            if (sets[set].datos[minimo].dirtyBit)
            {
                sets[set].datos[minimo].dirtyBit = false;
                esDirty = true;

                estadisticasV->caso = DIRTY_MISS;                    //VERBOSO
                estadisticasV->dirtyBit = 1;                         //VERBOSO
            }
            else
            {
                estadisticasV->caso = CLEAN_MISS;                    //VERBOSO
            }
            
            estadisticasV->lastUsed = sets[set].datos[minimo].numTransaccion;      //VERBOSO
            estadisticasV->prevTag = sets[set].datos[minimo].tag;    //VERBOSO
            estadisticasV->numerolinea = minimo;                     //VERBOSO
            estadisticasV->validBit = 1;                             //VERBOSO

            sets[set].datos[minimo].tag = tag;
            sets[set].datos[minimo].numTransaccion = sets->transacciones;

            if (strcmp(array[TIPO_OPERACION], ESCRITURA) == 0)
            {
                sets[set].datos[minimo].dirtyBit = true;
            }
        }
    }

    if (verboso != NULL && modoVerbosoActivo(verboso, cache))
    {
        imprimirLineaVerboso(estadisticasV, cache);
    }
    
    calcularEstadisticas(array, cache, esHit, esDirty);

    sets->transacciones++;
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