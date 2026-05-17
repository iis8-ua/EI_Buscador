#include "buscador.h"
#include <cstdio>
#include <cstring>

using namespace std;

static inline string extraerNombre(const string& ruta) {
    size_t slash = ruta.rfind('/');
    if (slash == string::npos) {
        slash = ruta.rfind('\\');
    }
    size_t ini = (slash == string::npos) ? 0 : slash + 1;
    size_t punto = ruta.rfind('.');
    size_t fin = (punto != string::npos && punto > ini) ? punto : ruta.size();
    return ruta.substr(ini, fin - ini);
}

void Buscador::RebuildCache() {
    const int D = static_cast<int>(indiceDocs.size());
    N_cache = D;

    cacheNombre.assign(D + 1, "");
    cacheLen.assign(D + 1, 1.0);
    scoresBuf.assign(D + 1, 0.0);
    docMarcado.assign(D + 1, false);
    docsActivos.clear();
    docsActivos.reserve(D);

    double suma = 0.0;
    for (const auto& kv : indiceDocs) {
        int id = kv.second.idDoc;
        if (id >= 1 && id <= D) {
            cacheNombre[id] = extraerNombre(kv.first);
            double len = static_cast<double>(kv.second.numPalSinParada);
            cacheLen[id] = (len >= 1.0) ? len : 1.0;
            suma+= cacheLen[id];
        }
    }
    avr_ld = (D > 0) ? suma / D : 1.0;
    if (avr_ld < 1.0) {
        avr_ld = 1.0;
    }
    RebuildDFRlog();
}

void Buscador::RebuildDFRlog() {
    const int D= N_cache;
    const double ca = c * avr_ld;
    cacheDFRlog.resize(D + 1, 0.0);
    for (int id = 1; id <= D; ++id){
        cacheDFRlog[id] = mylog2(1.0 + ca / cacheLen[id]);
    }
}

void Buscador::SortResultados() {
    sort(docsOrdenados.begin(), docsOrdenados.end(),
         [](const ResultadoRI& a, const ResultadoRI& b) {
             if (a.NumPregunta() != b.NumPregunta()){
                 return a.NumPregunta() < b.NumPregunta();
             }
             return a.VSimilitud() > b.VSimilitud();
         });
    docsOrdenadosYaSorted = true;
}

Buscador::Buscador()
    : IndexadorHash(), docsOrdenadosYaSorted(false),
      formSimilitud(0), c(2.0), k1(1.2), b(0.75),
      avr_ld(1.0), N_cache(0)
{}

Buscador::Buscador(const string& directorioIndexacion, const int& f)
    : IndexadorHash(directorioIndexacion),
      docsOrdenadosYaSorted(false),
      formSimilitud(f), c(2.0), k1(1.2), b(0.75),
      avr_ld(1.0), N_cache(0)
{
    RebuildCache();
}

Buscador::Buscador(const Buscador& busc)
    : IndexadorHash(busc),
      docsOrdenados(busc.docsOrdenados),
      docsOrdenadosYaSorted(busc.docsOrdenadosYaSorted),
      formSimilitud(busc.formSimilitud),
      c(busc.c), k1(busc.k1), b(busc.b),
      cacheNombre(busc.cacheNombre), cacheLen(busc.cacheLen),
      avr_ld(busc.avr_ld), N_cache(busc.N_cache),
      scoresBuf(busc.scoresBuf), docMarcado(busc.docMarcado),
      docsActivos(busc.docsActivos), cacheDFRlog(busc.cacheDFRlog)
{}

Buscador::~Buscador() {}

Buscador& Buscador::operator=(const Buscador& busc) {
    if (this != &busc) {
        IndexadorHash::operator=(busc);
        docsOrdenados= busc.docsOrdenados;
        docsOrdenadosYaSorted = busc.docsOrdenadosYaSorted;
        formSimilitud = busc.formSimilitud;
        c = busc.c; k1 = busc.k1; b = busc.b;
        cacheNombre= busc.cacheNombre;
        cacheLen = busc.cacheLen;
        avr_ld= busc.avr_ld;
        N_cache= busc.N_cache;
        scoresBuf= busc.scoresBuf;
        docMarcado= busc.docMarcado;
        docsActivos= busc.docsActivos;
        cacheDFRlog= busc.cacheDFRlog;
    }
    return *this;
}

int Buscador::DevolverFormulaSimilitud() const {
    return formSimilitud;
}

bool Buscador::CambiarFormulaSimilitud(const int& f) {
    if (f == 0 || f == 1) {
        formSimilitud = f;
        return true;
    }
    return false;
}

void Buscador::CambiarParametrosDFR(const double& kc) {
    c = kc;
    if (N_cache > 0) {
        RebuildDFRlog();
    }
}

double Buscador::DevolverParametrosDFR() const{
    return c;
}

void Buscador::CambiarParametrosBM25(const double& kk1, const double& kb) {
    k1 = kk1;
    b = kb;
}

void Buscador::DevolverParametrosBM25(double& kk1, double& kb) const{
    kk1 = k1;
    kb = b;
}

bool Buscador::BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta) {
    if (indicePregunta.empty()) {
        return false;
    }

    if (N_cache == 0) {
        return false;
    }

    const int N = N_cache;
    const int k= infPregunta.numTotalPalSinParada;
    const double inv_k = (k > 0) ? 1.0 / k : 1.0;

    for (long int id : docsActivos) {
        scoresBuf[id]  = 0.0;
        docMarcado[id] = false;
    }
    docsActivos.clear();

    const double bm_fixed = k1 * (1.0 - b);
    const double bm_bAvr= k1 * b / avr_ld;
    const double k1p1= k1 + 1.0;

    for (const auto& kvP : indicePregunta) {
        const string& termino = kvP.first;
        const InformacionTerminoPregunta& tp = kvP.second;

        auto itIdx = indice.find(termino);
        if (itIdx == indice.end()) {
            continue;
        }

        const InformacionTermino& infTerm = itIdx->second;
        const int ft_col = infTerm.ftc;
        const int nt= infTerm.numDocs();
        if (nt == 0) {
            continue;
        }
        const double wi_q = tp.ft * inv_k;

        if (formSimilitud == 0) {
            const double lam= static_cast<double>(ft_col) / N;
            const double log1lam= mylog2(1.0 + lam);
            const double logRatio = mylog2((1.0 + lam) / lam);
            const double ftcP1= ft_col + 1.0;
            const double ntD= static_cast<double>(nt);

            for (const auto& docPair : infTerm.l_docs) {
                const int docId = docPair.first;
                const int ft_d= docPair.second.ft;
                const double ld= cacheLen[docId];

                const double ft_star = ft_d * cacheDFRlog[docId];
                const double wi_d= (log1lam + ft_star * logRatio) * ftcP1 / (ntD * (ft_star + 1.0));

                if (!docMarcado[docId]) {
                    docMarcado[docId] = true;
                    docsActivos.push_back(docId);
                }
                scoresBuf[docId] += wi_q * wi_d;
            }

        }
        else {
            const double idf = mylog2((N - nt + 0.5) / (nt + 0.5));

            for (const auto& docPair : infTerm.l_docs) {
                const int docId =docPair.first;
                const int ft_d= docPair.second.ft;
                const double ld= cacheLen[docId];

                const double tf = (ft_d * k1p1) / (ft_d + bm_fixed + bm_bAvr * ld);

                if (!docMarcado[docId]) {
                    docMarcado[docId] = true;
                    docsActivos.push_back(docId);
                }
                scoresBuf[docId] += idf * tf;
            }
        }
    }

    for (long int docId : docsActivos){
        docsOrdenados.emplace_back(scoresBuf[docId], docId, numPregunta);
    }
    docsOrdenadosYaSorted = false;
    return true;
}

bool Buscador::Buscar(const int& numDocumentos) {
    docsOrdenados.clear();
    docsOrdenadosYaSorted = false;
    if (N_cache == 0 && !indiceDocs.empty()) {
        RebuildCache();
    }
    bool ok = BuscarPreguntaActual(numDocumentos, 0);
    if (ok) {
        SortResultados();
    }
    return ok;
}

bool Buscador::Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin) {
    docsOrdenados.clear();
    docsOrdenadosYaSorted = false;
    docsOrdenados.reserve(static_cast<size_t>(numPregFin - numPregInicio + 1) * N_cache);
    if (N_cache == 0 && !indiceDocs.empty()) {
        RebuildCache();
    }

    static const int BUF_SIZE = 65536;
    static char buf[BUF_SIZE];
    char numBuf[16];

    string ruta = dirPreguntas;
    const size_t baseLen = ruta.size();

    for (int np = numPregInicio; np <= numPregFin; ++np) {
        snprintf(numBuf, sizeof(numBuf), "%d.txt", np);
        ruta.resize(baseLen);
        ruta += numBuf;

        FILE* fp = fopen(ruta.c_str(), "rb");
        if (!fp) {
            continue;
        }
        string contenido;
        size_t leido;
        while ((leido = fread(buf, 1, BUF_SIZE, fp)) > 0){
            contenido.append(buf, leido);
        }
        fclose(fp);

        if (!IndexarPregunta(contenido)) {
            continue;
        }

        if (!BuscarPreguntaActual(numDocumentos, np)) {
            return false;
        }
    }

    SortResultados();
    return true;
}

void Buscador::EscribirResultados(ostream& out, const int& numDocumentos) {
    if (!docsOrdenadosYaSorted) {
        SortResultados();
    }

    const char* formula = (formSimilitud == 0) ? "DFR" : "BM25";
    string pregActual;
    DevuelvePregunta(pregActual);
    static const char* const conjStr = "ConjuntoDePreguntas";

    vector<int> posXPreg(85, 0);

    const size_t nRes = docsOrdenados.size();
    string salida;
    salida.reserve(nRes * 55);
    char linea[128];

    const char* const pregPtr= pregActual.c_str();
    const char* const fmlaPtr= formula;

    for (const ResultadoRI& r : docsOrdenados) {
        const int np  = r.NumPregunta();
        int& pos = posXPreg[np < 85 ? np : 0];
        if (pos >= numDocumentos) {
            ++pos;
            continue;
        }

        const char* nomPtr = (r.IdDoc() >= 1 && r.IdDoc() <= N_cache) ? cacheNombre[r.IdDoc()].c_str() : "?";
        const char* pregImp = (np == 0) ? pregPtr : conjStr;

        int n = snprintf(linea, sizeof(linea),
                         "%d %s %s %d %.6f %s\n",
                         np, fmlaPtr, nomPtr, pos, r.VSimilitud(), pregImp);

        if (n > 0) {
            salida.append(linea, static_cast<size_t>(n));
        }
        ++pos;
    }
    out.write(salida.data(), static_cast<streamsize>(salida.size()));
}

void Buscador::ImprimirResultadoBusqueda(const int& numDocumentos) {
    EscribirResultados(cout, numDocumentos);
}

bool Buscador::ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) {
    FILE* fp = fopen(nombreFichero.c_str(), "wb");
    if (!fp) {
        return false;
    }

    if (!docsOrdenadosYaSorted) {
        SortResultados();
    }

    const char* formula = (formSimilitud == 0) ? "DFR" : "BM25";
    string pregActual;
    DevuelvePregunta(pregActual);

    vector<int> posXPreg(85, 0);
    string salida;
    salida.reserve(docsOrdenados.size() * 60);
    char linea[256];

    for (const ResultadoRI& r : docsOrdenados) {
        const int np = r.NumPregunta();
        int& pos = posXPreg[np < 85 ? np : 0];
        if (pos >= numDocumentos) {
            ++pos;
            continue;
        }

        const string& nombre = (r.IdDoc() >= 1 && r.IdDoc() <= N_cache) ? cacheNombre[r.IdDoc()] : "?";
        const char* pregImp  = (np == 0) ? pregActual.c_str() : "ConjuntoDePreguntas";

        int n = snprintf(linea, sizeof(linea),
                         "%d %s %s %d %.6f %s\n",
                         np, formula, nombre.c_str(), pos, r.VSimilitud(), pregImp);

        if (n > 0) {
            salida.append(linea, static_cast<size_t>(n));
        }
        ++pos;
    }
    fwrite(salida.data(), 1, salida.size(), fp);
    fclose(fp);
    return true;
}