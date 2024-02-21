#ifndef CACHE_H
#define CACHE_H

#include "lib.h"

//ESTADISTICAS
typedef struct stats {
	int lecturas;
	int escrituras;
    int totalAccesos;
	int missesLectura;
    int missesEscritura;
	int totalMisses;
    int dirtyMissesLectura;
    int dirtyMissesEscritura;
	int cantBytesLeidos;
    int cantBytesEscritos;
    int tiempoAccesoEscritura;
	int tiempoAccesoLectura;
    float missRateTotal;
} stats_t;

//VIAS
typedef struct s_via {
    bool dirtyBit;
    int numTransaccion;
    unsigned int tag;
} via_t;

//ARRAY DE VIAS
typedef struct s_array {
    int cantidad;
    int tamanio;
    int transacciones;
    via_t* datos;
} array_t;

//CACHE
typedef struct s_cache {
	int tamanio;
	int cantidadSets;
    int lineasPorSet;
    int bytesPorBloque;
    array_t* sets;
    stats_t* estadisticas;
} cache_t;

//VERBOSO
typedef struct s_estadisticas_verboso {
    char* caso;
    int indice;
    int prevTag;
    int lastUsed;
    int validBit;
    int dirtyBit;
    uint32_t set;
    uint32_t tag;
    uint32_t numerolinea;
} estadisticas_verboso_t;

typedef struct s_verboso {
    bool activo;
    int inicio;
    int final;
} verboso_t;

verboso_t* inciarModoVerboso(int ini, int fin);
bool modoVerbosoActivo(verboso_t* verboso, cache_t* cache);
void liberarVerboso(verboso_t* verboso);

void limpiarEstadisticas(estadisticas_verboso_t* estadisticasV, int indice);
void imprimirLineaVerboso(estadisticas_verboso_t* estadisticasV, cache_t* cache);

cache_t* armarCache(int tamanio, int cantidadSets, int lineasPorSet);
void ingresarDato(char* array[], cache_t* cache, verboso_t* verboso, estadisticas_verboso_t* estadisticas);
void liberarCache(cache_t* cache);

array_t* crearArray(int cantidad, int capacidad);
void liberarArray(array_t* sets, cache_t* cache);

stats_t* crearEstadisticas();
void calcularEstadisticas(char* array[], cache_t* cache, bool esHit, bool esDirty);
void imprimirEstadisticas(cache_t* cache);
void liberarEstadisticas(stats_t* estadisticas);

//PROCESAMIENTO DE ARCHIVO
void procesarLineas(FILE *archivo, cache_t* cache, verboso_t* verboso);
#endif