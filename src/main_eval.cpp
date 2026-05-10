#include <iostream>
#include <string>
#include <list>
#include <sys/resource.h>
#include <ctime>
#include "tokenizador.h"
#include "indexadorHash.h"
#include "buscador.h"

using namespace std;

int main() {
    // 1. CONFIGURACIÓN DEL ÍNDICE
    // Para la evaluación del corpus TIME (inglés), el stemming debe ser 2 (Inglés) o 0 (Sin stemming)
    // El enunciado pide evaluar "con y sin stemming".
    // Aquí configuramos: Sin Stemming (0) y Modelo BM25 (1) como ejemplo.
    // CAMBIA '0' por '2' para probar CON stemming inglés.
    // CAMBIA el segundo '0' (modelo) por '1' para BM25 o '0' para DFR.

    int tipoStemmer = 0; // 0 = Sin stemming, 2 = Porter Inglés
    int modelo = 1;      // 0 = DFR, 1 = BM25

    cout << "Iniciando evaluación con Stemming: " << (tipoStemmer == 0 ? "NO" : "SI")
         << " y Modelo: " << (modelo == 0 ? "DFR" : "BM25") << endl;

    // Crear índice (si ya existe y no has cambiado stemming/modelo, puedes saltar esto y cargar directamente)
    // NOTA: Si cambias 'tipoStemmer', DEBES regenerar el índice.
    IndexadorHash b("./StopWordsEspanyol.txt", ". ,:", false, false,
                    "./indicePruebaEspanyol", tipoStemmer, false);

    // Solo indexar si el índice no existe o si has cambiado el stemming
    // Para ahorrar tiempo en pruebas, puedes comentar estas dos líneas si ya tienes el índice listo
    b.Indexar("ficherosTimes.txt");
    b.GuardarIndexacion();

    // 2. CARGAR EL BUSCADOR
    Buscador a("./indicePruebaEspanyol", modelo);

    // 3. EJECUTAR BÚSQUEDA EN LAS 83 PREGUNTAS
    string rutaPreguntas = "/home/israelizqdo/Escritorio/3Carrera/EI/Buscador/CorpusTime/Preguntas/";

    cout << "Buscando en las 83 preguntas..." << endl;
    if (!a.Buscar(rutaPreguntas, 423, 1, 83)) {
        cerr << "ERROR: La búsqueda falló." << endl;
        return 1;
    }

    // 4. GUARDAR RESULTADOS EN ARCHIVO (CRÍTICO PARA TREC_EVAL)
    string nombreArchivo = "fich_salida_buscador_alumno.txt";
    // Si quieres diferenciar los archivos por configuración, usa:
    // string nombreArchivo = "fich_salida_" + string(modelo == 0 ? "DFR" : "BM25") +
    //                        "_" + string(tipoStemmer == 0 ? "NoStem" : "Stem") + ".txt";

    if (!a.ImprimirResultadoBusqueda(423, nombreArchivo)) {
        cerr << "ERROR: No se pudo guardar el archivo de resultados." << endl;
        return 1;
    }

    cout << "Resultados guardados en: " << nombreArchivo << endl;
    cout << "Ejecución finalizada correctamente." << endl;

    return 0;
}