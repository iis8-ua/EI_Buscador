#ifndef BUSCADOR_H
#define BUSCADOR_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include "indexadorHash.h"

using namespace std;

// log2(x) = log(x) * M_LOG2E ? en la mayoría de libm log() usa ln nativo
// del FPU y es igual o más rápido que log2(). Función inline: el compilador
// la elimina completamente igual que haría con una macro, sin indirección.
static inline double mylog2(double x) { return log(x) * M_LOG2E; }

class ResultadoRI {
    friend ostream& operator<<(ostream& os, const ResultadoRI& res) {
        os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
        return os;
    }
public:
    ResultadoRI(const double& kvSimilitud, const long int& kidDoc, const int& np)
        : vSimilitud(kvSimilitud), idDoc(kidDoc), numPregunta(np) {}

    double   VSimilitud() const { return vSimilitud; }
    long int IdDoc()      const { return idDoc; }
    int      NumPregunta()const { return numPregunta; }

    bool operator<(const ResultadoRI& lhs) const {
        if (numPregunta == lhs.numPregunta)
            return (vSimilitud < lhs.vSimilitud);
        return (numPregunta > lhs.numPregunta);
    }

private:
    double   vSimilitud;
    long int idDoc;
    int      numPregunta;
};

class Buscador: public IndexadorHash {

    friend ostream& operator<<(ostream& s, const Buscador& p) {
        string preg;
        s << "Buscador: " << endl;
        if (p.DevuelvePregunta(preg))
            s << "\tPregunta indexada: " << preg << endl;
        else {
            s << "\tNo hay ninguna pregunta indexada" << endl;
            s << "\tDatos del indexador: " << endl << (IndexadorHash)p;
        }
        return s;
    }

public:
    Buscador(const string& directorioIndexacion, const int& f);
    Buscador(const Buscador&);
    ~Buscador();
    Buscador& operator=(const Buscador&);

    bool Buscar(const int& numDocumentos = 99999);
    bool Buscar(const string& dirPreguntas, const int& numDocumentos,
                const int& numPregInicio, const int& numPregFin);

    // Quitamos const: el método ordena docsOrdenados in-place antes de imprimir
    // para evitar la copia de ~840 KB que hacía la versión anterior.
    void ImprimirResultadoBusqueda(const int& numDocumentos = 99999);
    bool ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero);

    int  DevolverFormulaSimilitud() const;
    bool CambiarFormulaSimilitud(const int& f);

    void   CambiarParametrosDFR(const double& kc);
    double DevolverParametrosDFR() const;

    void CambiarParametrosBM25(const double& kk1, const double& kb);
    void DevolverParametrosBM25(double& kk1, double& kb) const;

private:
    Buscador();

    // Resultados: vector desordenado durante la búsqueda, ordenado in-place
    // antes de imprimir. Evita la copia de 35.000 × 24 bytes en cada impresión.
    vector<ResultadoRI> docsOrdenados;
    bool                docsOrdenadosYaSorted; // flag: evita re-sort innecesario

    // Parámetros de similitud
    int    formSimilitud;   // 0: DFR, 1: BM25
    double c;               // DFR
    double k1, b;           // BM25

    // Caché precalculada una sola vez en RebuildCache().
    // Vectores indexados directamente por idDoc (1-based): O(1) sin hashing,
    // datos contiguos ? todo en caché L1 con 423 docs (~7 KB total).
    vector<string>   cacheNombre;   // idDoc ? nombre sin ruta ni extensión
    vector<double>   cacheLen;      // idDoc ? numPalSinParada (ya como double)
    double           avr_ld;
    int              N_cache;

    // Buffers de scoring reutilizados entre llamadas a BuscarPreguntaActual.
    // Reset selectivo: solo se limpian las posiciones tocadas (docsActivos).
    vector<double>   scoresBuf;       // tamaño N+1, indexado por idDoc
    vector<bool>     docMarcado;      // tamaño N+1, marca qué docs tienen score
    vector<long int> docsActivos;     // idDocs tocados en la pasada actual

    // Tabla precalculada para DFR: cacheDFRlog[id] = log(1 + c*avr_ld / ld[id])
    // Solo depende de c y de la longitud del doc, ambas constantes entre preguntas.
    // Se recalcula si cambia c (CambiarParametrosDFR).
    vector<double>   cacheDFRlog;     // tamaño N+1, indexado por idDoc
    void RebuildDFRlog();             // reconstruye cacheDFRlog con el c actual

    void RebuildCache();
    // Ordena docsOrdenados in-place (llamado al final de Buscar y antes de imprimir)
    void SortResultados();
    bool BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta);
    void EscribirResultados(ostream& out, const int& numDocumentos);
};

#endif