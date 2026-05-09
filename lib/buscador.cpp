#include "buscador.h"

using namespace std;


Buscador::Buscador(const string& directorioIndexacion, const int& f)
    : IndexadorHash(directorioIndexacion),
      formSimilitud(f), c(2.0), k1(1.2), b(0.75)
{}

Buscador::Buscador()
    : IndexadorHash(),
      docsOrdenados(),
      formSimilitud(0),
      c(2.0),
      k1(1.2),
      b(0.75)
{}

Buscador::Buscador(const Buscador& busc)
    : IndexadorHash(busc),
      docsOrdenados(busc.docsOrdenados),
      formSimilitud(busc.formSimilitud),
      c(busc.c), k1(busc.k1), b(busc.b)
{}

Buscador::~Buscador() {}

Buscador& Buscador::operator=(const Buscador& busc) {
    if (this != &busc) {
        IndexadorHash::operator=(busc);
        docsOrdenados = busc.docsOrdenados;
        formSimilitud = busc.formSimilitud;
        c  = busc.c;
        k1 = busc.k1;
        b  = busc.b;
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

void Buscador::CambiarParametrosDFR(const double& kc){
    c =kc;
}

double Buscador::DevolverParametrosDFR() const{
    return c;
}

void Buscador::CambiarParametrosBM25(const double& kk1, const double& kb)  {
    k1 = kk1;
    b = kb;
}

void Buscador::DevolverParametrosBM25(double& kk1, double& kb) const {
    kk1 = k1;
    kb = b;
}


string Buscador::NombreDoc(long int id) const {
    for (const auto& kv : indiceDocs){
        if (kv.second.idDoc == id){
            return kv.first;
        }
    }
    return "?";
}

bool Buscador::BuscarPreguntaActual(const int& numDocumentos, const int& numPregunta) {

    // 1. Comprobar que hay terminos de pregunta
    if (indicePregunta.empty()) {
        cerr << "ERROR: No hay ninguna pregunta indexada con terminos validos." << endl;
        return false;
    }

    // 2. Datos de la coleccion
    int N = informacionColeccionDocs.numDocs;
    if (N == 0) {
        cerr << "ERROR: La coleccion esta vacia." << endl;
        return false;
    }

    double sumLen = 0.0;
    unordered_map<long int, int> idToLen;
    idToLen.reserve(indiceDocs.size());
    for (const auto& kv : indiceDocs) {
        idToLen[kv.second.idDoc] = kv.second.numPalSinParada;
        sumLen += kv.second.numPalSinParada;
    }
    double avr_ld = (N > 0) ? sumLen / N : 1.0;
    if (avr_ld < 1.0) {
        avr_ld = 1.0;
    }

    // 3. Numero de terminos distintos en la pregunta
    int k = infPregunta.numTotalPalSinParada;

    // 4. Acumular scores por documento
    unordered_map<long int, double> scores;

    for (const auto& kvP : indicePregunta) {
        const string& termino = kvP.first;
        const InformacionTerminoPregunta& tp = kvP.second;

        InformacionTermino infTerm;
        if (!Devuelve(termino, infTerm)) {
            continue;
        }

        int ft_col = infTerm.ftc;
        int nt = infTerm.numDocs();
        int ft_q = tp.ft;

        double wi_q = static_cast<double>(ft_q) / k;

        // Iterar documentos que contienen el termino
        for (const auto& docPair : infTerm.l_docs) {
            long int docId = docPair.first;
            int ft_d  = docPair.second.ft;

            double ld = static_cast<double>(idToLen.count(docId) ? idToLen[docId] : 1);
            if (ld < 1.0) {
                ld = 1.0;
            }

            double score_term = 0.0;

            if (formSimilitud == 0) {
                // DFR: wi_d = (log2(1+lam) + ft* * log2((1+lam)/lam)) * (ft_col+1) / (nt*(ft*+1))
                // lambda_t = ft_col / N
                double lam = static_cast<double>(ft_col) / N;
                double ft_star = ft_d * log2(1.0 + c * avr_ld / ld);
                double wi_d = (log2(1.0 + lam) + ft_star * log2((1.0 + lam) / lam))
                                 * (ft_col + 1.0) / (nt * (ft_star + 1.0));
                score_term = wi_q * wi_d;
            }
            else {
                // BM25
                double idf = log2((N - nt + 0.5) / (nt + 0.5));
                double tf  = (static_cast<double>(ft_d) * (k1 + 1.0)) /
                             (ft_d + k1 * (1.0 - b + b * ld / avr_ld));
                score_term = idf * tf;
            }
            scores[docId] += score_term;
        }
    }

    // 5. Insertar en docsOrdenados
    for (const auto& kv : scores){
        docsOrdenados.push(ResultadoRI(kv.second, kv.first, numPregunta));
    }
    return true;
}


bool Buscador::Buscar(const int& numDocumentos) {
    while (!docsOrdenados.empty()) {
        docsOrdenados.pop();
    }
    return BuscarPreguntaActual(numDocumentos, 0);
}


bool Buscador::Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin) {
    while (!docsOrdenados.empty()) {
        docsOrdenados.pop();
    }

    for (int np = numPregInicio; np <= numPregFin; ++np) {
        string fich = dirPreguntas + to_string(np) + ".txt";
        ifstream f(fich.c_str());
        if (!f) {
            cerr << "ERROR: No se puede abrir " << fich << endl;
            continue;
        }
        string contenido((istreambuf_iterator<char>(f)),istreambuf_iterator<char>());
        f.close();

        if (!IndexarPregunta(contenido)) {
            continue;
        }
        if (!BuscarPreguntaActual(numDocumentos, np)) {
            cerr << "ERROR: Fallo en busqueda pregunta " << np << endl;
            return false;
        }
    }
    return true;
}

void Buscador::EscribirResultados(ostream& out, const int& numDocumentos) const {
    string formula = (formSimilitud == 0) ? "DFR" : "BM25";

    string pregActual;
    DevuelvePregunta(pregActual);

    priority_queue<ResultadoRI> copia = docsOrdenados;
    vector<ResultadoRI> todos;
    while (!copia.empty()) {
        todos.push_back(copia.top());
        copia.pop();
    }

    unordered_map<int, int> posXPreg;

    for (const ResultadoRI& r : todos) {
        int np  = r.NumPregunta();
        int pos = posXPreg[np];

        if (pos >= numDocumentos) {
            posXPreg[np]++;
            continue;
        }

        string nomCompleto = NombreDoc(r.IdDoc());
        size_t slash = nomCompleto.rfind('/');

        if (slash == string::npos) {
            slash = nomCompleto.rfind('\\');
        }

        string nombre = (slash != string::npos) ? nomCompleto.substr(slash + 1) : nomCompleto;
        size_t punto  = nombre.rfind('.');

        if (punto != string::npos) {
            nombre = nombre.substr(0, punto);
        }

        string pregImp = (np == 0) ? pregActual : "ConjuntoDePreguntas";

        out << np << " " << formula << " " << nombre
            << " " << pos << " "
            << fixed << setprecision(6) << r.VSimilitud()
            << " " << pregImp << "\n";

        posXPreg[np]++;
    }
}

void Buscador::ImprimirResultadoBusqueda(const int& numDocumentos) const {
    EscribirResultados(cout, numDocumentos);
}

bool Buscador::ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) const {
    ofstream f(nombreFichero.c_str());
    if (!f) {
        cerr << "ERROR: No se puede crear el fichero " << nombreFichero << endl;
        return false;
    }
    EscribirResultados(f, numDocumentos);
    return true;
}