#ifndef BUSCADOR_H
#define BUSCADOR_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <queue>
#include "indexadorHash.h"

using namespace std;

static inline double mylog2(double x) {
    return log(x) * M_LOG2E;
}

class ResultadoRI {
    friend ostream& operator<<(ostream& os, const ResultadoRI& res) {
        os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
        return os;
    }

public:
    ResultadoRI(const double& kvSimilitud, const long int& kidDoc, const int& np)
        : vSimilitud(kvSimilitud), idDoc(kidDoc), numPregunta(np) {}

    double VSimilitud() const {
        return vSimilitud;
    }

    long int IdDoc() const {
        return idDoc;
    }

    int NumPregunta() const {
        return numPregunta;
    }

    bool operator<(const ResultadoRI& lhs) const {
        if (numPregunta == lhs.numPregunta){
            return (vSimilitud < lhs.vSimilitud);
        }
        return (numPregunta > lhs.numPregunta);
    }

private:
    double vSimilitud;
    long int idDoc;
    int numPregunta;
};

class Buscador: public IndexadorHash {

    friend ostream& operator<<(ostream& s, const Buscador& p) {
        string preg;
        s << "Buscador: " << endl;
        if (p.DevuelvePregunta(preg)){
            s << "\tPregunta indexada: " << preg << endl;
        }
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
    bool Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin);
    void ImprimirResultadoBusqueda(const int& numDocumentos = 99999);
    bool ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero);
    int  DevolverFormulaSimilitud() const;
    bool CambiarFormulaSimilitud(const int& f);
    void CambiarParametrosDFR(const double& kc);
    double DevolverParametrosDFR() const;
    void CambiarParametrosBM25(const double& kk1, const double& kb);
    void DevolverParametrosBM25(double& kk1, double& kb) const;

private:
    Buscador();

    vector<ResultadoRI> docsOrdenados;
    bool docsOrdenadosYaSorted;
    int formSimilitud;
    double c;
    double k1, b;
    vector<string> cacheNombre;
    vector<double> cacheLen;
    double  avr_ld;
    int N_cache;
    vector<double> scoresBuf;
    vector<uint8_t> docMarcado;
    vector<long int> docsActivos;
    vector<double> cacheDFRlog;

    void RebuildDFRlog();
    void RebuildCache();
    void SortResultados();
    bool BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta);
    void EscribirResultados(ostream& out, const int& numDocumentos);
};

#endif