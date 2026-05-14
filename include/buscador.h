#ifndef BUSCADOR_H
#define BUSCADOR_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include "indexadorHash.h"

using namespace std;

class ResultadoRI {
    friend ostream& operator<<(ostream& os, const ResultadoRI& res) {
        os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
        return os;
    }
public:
    ResultadoRI(const double& kvSimilitud, const long int& kidDoc, const int& np)
        : vSimilitud(kvSimilitud), idDoc(kidDoc), numPregunta(np) {}

    double VSimilitud() const { return vSimilitud; }
    long int IdDoc()    const { return idDoc; }
    int NumPregunta()   const { return numPregunta; }

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

    void ImprimirResultadoBusqueda(const int& numDocumentos = 99999) const;
    bool ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) const;

    int  DevolverFormulaSimilitud() const;
    bool CambiarFormulaSimilitud(const int& f);

    void   CambiarParametrosDFR(const double& kc);
    double DevolverParametrosDFR() const;

    void CambiarParametrosBM25(const double& kk1, const double& kb);
    void DevolverParametrosBM25(double& kk1, double& kb) const;

private:
    Buscador();

    // ?? resultados ??????????????????????????????????????????????????????????
    // Vector en lugar de priority_queue: acumulamos todos los resultados de
    // todas las preguntas y ordenamos una sola vez en ImprimirResultadoBusqueda.
    // Con 83 preguntas × 423 docs = ~35.000 elementos, un sort final es más
    // rápido que 35.000 push al heap con sus rebalanceos.
    vector<ResultadoRI> docsOrdenados;

    // ?? parámetros de similitud ??????????????????????????????????????????????
    int    formSimilitud;   // 0: DFR, 1: BM25
    double c;               // DFR
    double k1, b;           // BM25

    // ?? CACHÉ precalculada (se llena en RebuildCache) ????????????????????????
    // Los idDoc son enteros 1..N, por lo que usamos vectores indexados
    // directamente: acceso O(1) sin hashing, datos contiguos en memoria ? L1.

    // idDoc (1-based) ? nombre sin ruta ni extensión.  índice 0 no se usa.
    vector<string> cacheNombre;   // tamaño N+1

    // idDoc (1-based) ? numPalSinParada (como double para evitar cast en scoring)
    vector<double> cacheLen;      // tamaño N+1

    // media de longitudes (palabras sin parada)
    double avr_ld;
    // número de docs
    int    N_cache;

    // Acumulador de scores por documento en BuscarPreguntaActual.
    // vector<double> de tamaño N+1, indexado por idDoc directamente.
    // Se reutiliza entre llamadas: se resetea solo en las posiciones tocadas.
    vector<double>   scoresBuf;   // tamaño N+1, valores inicializados a 0
    vector<bool>     docMarcado;  // tamaño N+1, valores inicializados a false
    vector<long int> docsActivos; // lista de idDocs con score != 0 en la pasada actual

    // Construye la caché a partir de indiceDocs. Se llama una sola vez al cargar.
    void RebuildCache();

    // Núcleo de búsqueda para la pregunta ya indexada
    bool BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta);

    // Escribe resultados en un stream de salida
    void EscribirResultados(ostream& out, const int& numDocumentos) const;
};

#endif