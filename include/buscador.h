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
#include "indexadorHash.h"

using namespace std;

class ResultadoRI {
    friend ostream& operator<<(ostream& os, const ResultadoRI& res) {
        os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
        return os;
    }
public:
    ResultadoRI(const double& kvSimilitud, const long int& kidDoc, const int& np): vSimilitud(kvSimilitud), idDoc(kidDoc), numPregunta(np) {}

    double VSimilitud() const {
        return vSimilitud;
    }

    long int IdDoc() const {
        return idDoc;
    }

    int NumPregunta() const {
        return numPregunta;
    }

    // priority_queue es max-heap: el operador < define el orden
    // Queremos que salgan primero los de menor numPregunta y,
    // dentro de la misma pregunta, los de mayor vSimilitud
    bool operator<(const ResultadoRI& lhs) const {
        if (numPregunta == lhs.numPregunta){
            return (vSimilitud < lhs.vSimilitud);
        }
        else{
            return (numPregunta > lhs.numPregunta);
        }
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
        if(p.DevuelvePregunta(preg)){
            s << "\tPregunta indexada: " << preg << endl;
        }
        else{
            s << "\tNo hay ninguna pregunta indexada" << endl;
            s << "\tDatos del indexador: " << endl << (IndexadorHash) p;
        }
        return s;
    }

public:
    Buscador(const string& directorioIndexacion, const int& f);
    // Constructor para inicializar Buscador a partir de la indexaci?n generada previamente y almacenada en "directorioIndexacion". En caso que no exista el directorio o que no contenga los datos de la indexaci?n se enviar? a cerr la excepci?n correspondiente y se continuar? la ejecuci?n del programa manteniendo los ?ndices vac?os
    // Inicializar? la variable privada "formSimilitud" a "f" y las constantes de cada modelo: "c = 2;  k1 = 1.2;  b = 0.75;"

    Buscador(const Buscador&);

    ~Buscador();

    Buscador& operator= (const Buscador&);

    // En los m?todos de "Buscar" solo se evaluar?n TODOS los documentos que contengan alguno de los t?rminos de la pregunta (tras eliminar las palabras de parada).
    bool Buscar(const int& numDocumentos = 99999);
    // Devuelve true si en IndexadorHash.pregunta hay indexada una pregunta no vac?a con alg?n t?rmino con contenido, y si sobre esa pregunta se finaliza la b?squeda correctamente con la f?rmula de similitud indicada en la variable privada "formSimilitud".
    // Por ejemplo, devuelve falso si no finaliza la b?squeda por falta de memoria, mostrando el mensaje de error correspondiente, e indicando el documento y t?rmino en el que se ha quedado.
    // Se guardar?n los primeros "numDocumentos" documentos m?s relevantes en la variable privada "docsOrdenados" en orden decreciente seg?n la relevancia sobre la pregunta (se vaciar? previamente el contenido de esta variable antes de realizar la b?squeda). Se almacenar?n solo los documentos que compartan alg?n t?rmino (no de parada) con la query (aunque ese n?mero de documentos sea inferior a "numDocumentos"). Como n?mero de pregunta en "ResultadoRI.numPregunta" se almacenar? el valor 0. En caso de que no se introduzca el par?metro "numDocumentos", entonces dicho par?metro se inicializar? a 99999)

    bool Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin);
    // Realizar? la b?squeda entre el n?mero de pregunta "numPregInicio" y "numPregFin", ambas preguntas incluidas. El corpus de preguntas estar? en el directorio "dirPreguntas", y tendr? la estructura de cada pregunta en un fichero independiente, de nombre el n?mero de pregunta, y extensi?n ".txt" (p.ej. 1.txt 2.txt 3.txt ... 83.txt). Esto significa que habr? que indexar cada pregunta por separado y ejecutar una b?squeda por cada pregunta a?adiendo los resultados de cada pregunta (junto con su n?mero de pregunta) en la variable privada "docsOrdenados". Asimismo, se supone que previamente se mantendr? la indexaci?n del corpus.
    // Se guardar?n los primeros "numDocumentos" documentos m?s relevantes para cada pregunta en la variable privada "docsOrdenados". Se almacenar?n solo los documentos que compartan alg?n t?rmino (no de parada) con la query (aunque ese n?mero de documentos sea inferior a "numDocumentos").
    // La b?squeda se realiza con la f?rmula de similitud indicada en la variable privada "formSimilitud".
    // Devuelve falso si no finaliza la b?squeda (p.ej. por falta de memoria), mostrando el mensaje de error correspondiente, indicando el documento, pregunta y t?rmino en el que se ha quedado.

    void ImprimirResultadoBusqueda(const int& numDocumentos = 99999) const;
    // Imprimir? por pantalla los resultados de la ?ltima b?squeda (un n?mero M?XIMO de "numDocumentos" (en caso de que no se introduzca el par?metro "numDocumentos", entonces dicho par?metro se inicializar? a 99999) por cada pregunta, entre los que tienen alg?n t?rmino de la pregunta), los cuales estar?n almacenados en la variable privada "docsOrdenados" en orden decreciente seg?n la relevancia sobre la pregunta, en el siguiente formato (una l?nea por cada documento):
    //NumPregunta FormulaSimilitud NomDocumento Posicion PuntuacionDoc PreguntaIndexada
    // Donde:
    //NumPregunta ser?a el n?mero de pregunta almacenado en "ResultadoRI.numPregunta"
    //FormulaSimilitud ser?a: "DFR" si la variable privada "formSimilitud == 0"; "BM25" si es 1.
    //NomDocumento ser?a el nombre 	del documento almacenado en la indexaci?n (habr? que extraer el nombre del documento a partir de "ResultadoRI.idDoc", pero sin el directorio donde est? almacenado ni la extensi?n del archivo)
    //Posicion empezar?a desde 0 (indicando el documento m?s relevante para la pregunta) increment?ndose por cada documento (ordenado por relevancia). Se imprimir? un m?ximo de l?neas de "numDocumentos" (es decir, el m?ximo valor de este campo ser? "numDocumentos - 1")
    //PuntuacionDoc ser?a el valor num?rico de la f?rmula de similitud empleada almacenado en "ResultadoRI.vSimilitud". Se mostrar?n los decimales con el punto en lugar de con la coma.
    //PreguntaIndexada se corresponde con IndexadorHash.pregunta si "ResultadoRI.numPregunta == 0". En caso contrario se imprimir? "ConjuntoDePreguntas"
    // Por ejemplo:
    //1 BM25 EFE19950609-05926 0 64.7059 ConjuntoDePreguntas
    //1 BM25 EFE19950614-08956 1 63.9759 ConjuntoDePreguntas
    //1 BM25 EFE19950610-06424 2 62.6695 ConjuntoDePreguntas
    //2 BM25 EFE19950610-00234 0 0.11656233535972 ConjuntoDePreguntas
    //2 BM25 EFE19950610-06000 1 0.10667871616613 ConjuntoDePreguntas
    // Este archivo deber?a usarse con la utilidad "trec_eval -q -o frelevancia_trec_eval_TIME.txt fich_salida_buscador.txt > fich_salida_trec_eval.res", que se consigue compilando "make" del directorio "trec_eval_8_1_linux" de "materiales_buscador.tgz", para obtener los datos de precisi?n y cobertura

    bool ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) const;
    // Lo mismo que "ImprimirResultadoBusqueda()" pero guardando la salida en el fichero de nombre "nombreFichero"
    // Devolver? false si no consigue crear correctamente el archivo

    int DevolverFormulaSimilitud() const;
    // Devuelve el valor del campo privado "formSimilitud"

    bool CambiarFormulaSimilitud(const int& f);
    // Cambia el valor de "formSimilitud" a "f" si contiene un valor correcto (f == 0 || f == 1);
    // Devolver? false si "f" no contiene un valor correcto, en cuyo caso no cambiar?a el valor anterior de "formSimilitud"

    void CambiarParametrosDFR(const double& kc);
    // Cambia el valor de "c = kc"

    double DevolverParametrosDFR() const;
    // Devuelve el valor de "c"

    void CambiarParametrosBM25(const double& kk1, const double& kb);
    // Cambia el valor de "k1 = kk1; b = kb;"

    void DevolverParametrosBM25(double& kk1, double& kb) const;
    // Devuelve el valor de "k1" y "b"

private:
    Buscador();
    // Este constructor se pone en la parte privada porque no se permitir? crear un buscador sin inicializarlo convenientemente a partir de una indexaci?n.
    // Se inicializar? con todos los campos vac?os y la variable privada "formSimilitud" con valor 0 y las constantes de cada modelo: "c = 2;  k1 = 1.2; b = 0.75"

    priority_queue< ResultadoRI > docsOrdenados;
    // Contendr? los resultados de la ?ltima b?squeda realizada en orden decreciente seg?n la relevancia sobre la pregunta. El tipo "priority_queue" podr? modificarse por cuestiones de eficiencia. La clase "ResultadoRI" aparece en la secci?n "Ejemplo de modo de uso de la cola de prioridad de STL"

    int formSimilitud;
    // 0: DFR, 1: BM25

    double c;
    // Constante del modelo DFR

    double k1;
    // Constante del modelo BM25

    double b;
    // Constante del modelo BM25

    // Devuelve el nombre de documento (sin directorio ni extensión) dado su idDoc
    string NombreDoc(long int idDoc) const;

    // Núcleo de la búsqueda: rellena docsOrdenados para la pregunta actual
    // Devuelve false si falla
    bool BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta);

    // Escribe los resultados en un stream
    void EscribirResultados(ostream& out, const int& numDocumentos) const;
};

#endif
