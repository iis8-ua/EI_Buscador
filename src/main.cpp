#include <iostream>
#include <string>
#include "tokenizador.h"
#include "indexadorHash.h"
#include "buscador.h"

using namespace std;

int main() {

    // === AJUSTA ESTAS RUTAS ===
    string dirPreguntas  = "./CorpusTime/Preguntas/";
    string ficherosTimes = "ficherosTimes.txt";
    // ==========================

    // --- 1. Sin stemming: indexar una sola vez ---
    {
        IndexadorHash idx("./StopWordsIngles.txt", "", true, true,
                          "./indice_NoStem", 0, false);
        idx.Indexar(ficherosTimes);
        idx.GuardarIndexacion();
    }

    // --- 2. Con stemming (Porter inglés): indexar una sola vez ---
    {
        IndexadorHash idx("./StopWordsIngles.txt", "", true, true,
                          "./indice_Stem", 2, false);
        idx.Indexar(ficherosTimes);
        idx.GuardarIndexacion();
    }

    // --- Combinación 1: DFR + Sin stemming ---
    {
        Buscador a("./indice_NoStem", 0);
        a.Buscar(dirPreguntas, 423, 1, 83);
        a.ImprimirResultadoBusqueda(423, "fich_salida_DFR_NoStem.txt");
        cout << "OK: fich_salida_DFR_NoStem.txt" << endl;
    }

    // --- Combinación 2: BM25 + Sin stemming ---
    {
        Buscador a("./indice_NoStem", 1);
        a.Buscar(dirPreguntas, 423, 1, 83);
        a.ImprimirResultadoBusqueda(423, "fich_salida_BM25_NoStem.txt");
        cout << "OK: fich_salida_BM25_NoStem.txt" << endl;
    }

    // --- Combinación 3: DFR + Con stemming ---
    {
        Buscador a("./indice_Stem", 0);
        a.Buscar(dirPreguntas, 423, 1, 83);
        a.ImprimirResultadoBusqueda(423, "fich_salida_DFR_Stem.txt");
        cout << "OK: fich_salida_DFR_Stem.txt" << endl;
    }

    // --- Combinación 4: BM25 + Con stemming ---
    {
        Buscador a("./indice_Stem", 1);
        a.Buscar(dirPreguntas, 423, 1, 83);
        a.ImprimirResultadoBusqueda(423, "fich_salida_BM25_Stem.txt");
        cout << "OK: fich_salida_BM25_Stem.txt" << endl;
    }

    return 0;
}