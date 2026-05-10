#ifndef BUSCADOR_H
#define BUSCADOR_H

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <unordered_map>
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
    priority_queue<ResultadoRI> docsOrdenados;

    // ?? parámetros de similitud ??????????????????????????????????????????????
    int    formSimilitud;   // 0: DFR, 1: BM25
    double c;               // DFR
    double k1, b;           // BM25

    // ?? CACHÉ precalculada (se llena en RebuildCache) ????????????????????????
    // idDoc ? nombre de fichero sin directorio ni extensión
    unordered_map<long int, string> cacheNombre;
    // idDoc ? numPalSinParada
    unordered_map<long int, int>    cacheLen;
    // media de longitudes (palabras sin parada)
    double avr_ld;
    // número de docs (igual que informacionColeccionDocs.numDocs, pero cacheado)
    int    N_cache;

    // Construye/reconstruye los tres mapas de caché a partir de indiceDocs.
    // Se llama tras cargar la indexación y cada vez que pueda haber cambiado.
    void RebuildCache();

    // Núcleo de búsqueda para la pregunta ya indexada
    bool BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta);

    // Escribe resultados en un stream de salida
    void EscribirResultados(ostream& out, const int& numDocumentos) const;
};

#endif