#include "indexadorHash.h"
using namespace std;

IndexadorHash::IndexadorHash(const string& fichStopWords, const string& delimitadores,
                             const bool& detectComp, const bool& minuscSinAcentos,
                             const string& dirIndice, const int& tStemmer,
                             const bool& almPosTerm)
    : contadorID(0)
{
    ficheroStopWords=fichStopWords;
    directorioIndice=dirIndice;
    tipoStemmer=tStemmer;
    almacenarPosTerm=almPosTerm;

    tok.DelimitadoresPalabra(delimitadores);
    tok.CasosEspeciales(detectComp);
    tok.PasarAminuscSinAcentos(minuscSinAcentos);

    CargarStopWords(fichStopWords);

    indice.reserve(1 << 16);
    indice.max_load_factor(0.25f);
    indiceDocs.reserve(1 << 10);
}

IndexadorHash::IndexadorHash(const string& directorioIndexacion)
    : contadorID(0)
{
    ficheroStopWords="";
    directorioIndice="";
    tipoStemmer=0;
    almacenarPosTerm=false;
    pregunta="";

    RecuperarIndexacion(directorioIndexacion);
}

IndexadorHash::IndexadorHash(const IndexadorHash& t)
    : indice(t.indice),
      indiceDocs(t.indiceDocs),
      informacionColeccionDocs(t.informacionColeccionDocs),
      pregunta(t.pregunta),
      indicePregunta(t.indicePregunta),
      infPregunta(t.infPregunta),
      stopWords(t.stopWords),
      ficheroStopWords(t.ficheroStopWords),
      tok(t.tok),
      directorioIndice(t.directorioIndice),
      tipoStemmer(t.tipoStemmer),
      almacenarPosTerm(t.almacenarPosTerm),
      contadorID(t.contadorID)
{}

IndexadorHash::~IndexadorHash() {
    indice.clear();
    indiceDocs.clear();
    indicePregunta.clear();
    stopWords.clear();
    pregunta.clear();
    ficheroStopWords.clear();
    directorioIndice.clear();
}

IndexadorHash& IndexadorHash::operator=(const IndexadorHash& t) {
    if (this != &t) {
        indice= t.indice;
        indiceDocs= t.indiceDocs;
        informacionColeccionDocs = t.informacionColeccionDocs;
        pregunta= t.pregunta;
        indicePregunta= t.indicePregunta;
        infPregunta= t.infPregunta;
        stopWords= t.stopWords;
        ficheroStopWords= t.ficheroStopWords;
        tok= t.tok;
        directorioIndice= t.directorioIndice;
        tipoStemmer = t.tipoStemmer;
        almacenarPosTerm= t.almacenarPosTerm;
        contadorID= t.contadorID;
    }
    return *this;
}

void IndexadorHash::CargarStopWords(const string& fichStopWords) {
    ifstream archivo(fichStopWords.c_str());
    if (!archivo) {
        return;
    }

    stopWords.clear();
    stopWordsFiltrado.clear();
    string linea;
    vector<string> tokens;
    stemmerPorter sp;

    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        tok.TokenizarRapido(linea, tokens);
        for (const string& t : tokens) {
            stopWords.insert(t);
            string termino = t;
            if (tipoStemmer != 0) {
                sp.stemmer(termino, tipoStemmer);
            }
            stopWordsFiltrado.insert(termino);
        }
    }
    archivo.close();
}

int IndexadorHash::NumPalIndexadas() const {
    return static_cast<int>(indice.size());
}
string IndexadorHash::DevolverFichPalParada() const {
    return ficheroStopWords;
}
int IndexadorHash::NumPalParada() const {
    return static_cast<int>(stopWords.size());
}
string IndexadorHash::DevolverDelimitadores() const {
    return tok.DelimitadoresPalabra();
}
bool IndexadorHash::DevolverCasosEspeciales() const {
    return const_cast<Tokenizador&>(tok).CasosEspeciales();
}
bool IndexadorHash::DevolverPasarAminuscSinAcentos() const {
    return const_cast<Tokenizador&>(tok).PasarAminuscSinAcentos();
}
bool IndexadorHash::DevolverAlmacenarPosTerm() const {
    return almacenarPosTerm;
}
string IndexadorHash::DevolverDirIndice() const {
    return directorioIndice;
}
int IndexadorHash::DevolverTipoStemming() const {
    return tipoStemmer;
}

void IndexadorHash::ListarPalParada() const {
    for (unordered_set<string>::const_iterator it = stopWords.begin(); it != stopWords.end(); ++it) {
        cout << *it << endl;
    }
}

void IndexadorHash::ListarInfColeccDocs() const {
    cout << informacionColeccionDocs << endl;
}

void IndexadorHash::ListarTerminos() const {
    for (unordered_map<string,InformacionTermino>::const_iterator it = indice.begin(); it != indice.end(); ++it) {
        cout << it->first << '\t' << it->second << endl;
    }
}

bool IndexadorHash::ListarTerminos(const string& nomDoc) const {
    unordered_map<string,InfDoc>::const_iterator itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        return false;
    }
    int idDelDoc = itDoc->second.idDoc;

    for (unordered_map<string,InformacionTermino>::const_iterator itTerm = indice.begin(); itTerm != indice.end(); ++itTerm) {
        if (itTerm->second.find(idDelDoc) != 0) {
            cout << itTerm->first << '\t' << itTerm->second << endl;
        }
    }
    return true;
}

void IndexadorHash::ListarDocs() const {
    for (unordered_map<string,InfDoc>::const_iterator it = indiceDocs.begin(); it != indiceDocs.end(); ++it) {
        cout << it->first << '\t' << it->second << endl;
    }
}

bool IndexadorHash::ListarDocs(const string& nomDoc) const {
    unordered_map<string,InfDoc>::const_iterator it = indiceDocs.find(nomDoc);
    if (it != indiceDocs.end()) {
        cout << it->first << '\t' << it->second << endl;
        return true;
    }
    return false;
}

void IndexadorHash::VaciarIndiceDocs() {
    indice.clear();
    indiceDocs.clear();
    informacionColeccionDocs = InfColeccionDocs();
}

void IndexadorHash::VaciarIndicePreg() {
    pregunta.clear();
    indicePregunta.clear();
    infPregunta = InformacionPregunta();
}
bool IndexadorHash::PrepararTermino(const string& word, Tokenizador& tok, int tipoStemmer, string& termino) const {
    list<string> tokens;
    tok.Tokenizar(word, tokens);
    if (tokens.empty()) {
        return false;
    }
    termino = tokens.front();
    if (tipoStemmer != 0) {
        stemmerPorter sp;
        sp.stemmer(termino, tipoStemmer);
    }
    return true;
}

bool IndexadorHash::Existe(const string& word) const {
    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)){
        return false;
    }
    return indice.find(termino) != indice.end();
}

bool IndexadorHash::Devuelve(const string& word, InformacionTermino& inf) const {
    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)) {
        inf = InformacionTermino();
        return false;
    }

    unordered_map<string,InformacionTermino>::const_iterator it = indice.find(termino);
    if (it == indice.end()) {
        inf = InformacionTermino();
        return false;
    }

    inf = it->second;
    return true;
}

bool IndexadorHash::Devuelve(const string& word, const string& nomDoc, InfTermDoc& infDoc) const {
    unordered_map<string,InfDoc>::const_iterator itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        infDoc = InfTermDoc();
        return false;
    }
    int idDoc = itDoc->second.idDoc;

    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)) {
        infDoc = InfTermDoc();
        return false;
    }

    unordered_map<string,InformacionTermino>::const_iterator itTerm = indice.find(termino);
    if (itTerm != indice.end()) {
        const InfTermDoc* p = itTerm->second.find(idDoc);
        if (p != 0) {
            infDoc = *p;
            return true;
        }
    }

    infDoc = InfTermDoc();
    return false;
}

bool IndexadorHash::DevuelvePregunta(string& preg) const {
    if (indicePregunta.empty()) {
        return false;
    }
    preg = pregunta;
    return true;
}

bool IndexadorHash::DevuelvePregunta(InformacionPregunta& inf) const {
    if (indicePregunta.empty()) {
        inf = InformacionPregunta();
        return false;
    }
    inf = infPregunta;
    return true;
}

bool IndexadorHash::DevuelvePregunta(const string& word, InformacionTerminoPregunta& inf) const {
    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)) {
        inf = InformacionTerminoPregunta();
        return false;
    }

    unordered_map<string,InformacionTerminoPregunta>::const_iterator it =indicePregunta.find(termino);
    if (it == indicePregunta.end()) {
        inf = InformacionTerminoPregunta();
        return false;
    }

    inf = it->second;
    return true;
}

bool IndexadorHash::IndexarPregunta(const string& preg) {
    VaciarIndicePreg();
    pregunta = preg;

    list<string> tokens;
    tok.Tokenizar(preg, tokens);

    if (tokens.empty()) {
        return false;
    }

    stemmerPorter sp;
    int pos = 0;

    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        infPregunta.numTotalPal++;

        string termino = *it;
        if (tipoStemmer != 0) {
            sp.stemmer(termino, tipoStemmer);
        }

        if (stopWordsFiltrado.find(termino) != stopWordsFiltrado.end()) {
            pos++;
            continue;
        }

        infPregunta.numTotalPalSinParada++;
        InformacionTerminoPregunta& itp = indicePregunta[termino];

        if (itp.ft == 0) {
            infPregunta.numTotalPalDiferentes++;
        }
        itp.ft++;

        if (almacenarPosTerm) {
            itp.posTerm.push_back(pos);
        }
        pos++;
    }

    return !indicePregunta.empty();
}

bool IndexadorHash::BorraDoc(const string& nomDoc) {
    unordered_map<string,InfDoc>::iterator itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        return false;
    }

    int idABorrar = itDoc->second.idDoc;

    informacionColeccionDocs.numDocs--;
    informacionColeccionDocs.numTotalPal-= itDoc->second.numPal;
    informacionColeccionDocs.numTotalPalSinParada-= itDoc->second.numPalSinParada;
    informacionColeccionDocs.tamBytes-= itDoc->second.tamBytes;

    for (unordered_map<string,InformacionTermino>::iterator itTerm = indice.begin(); itTerm != indice.end(); ) {

        InfTermDoc* p = itTerm->second.find(idABorrar);
        if (p != 0) {
            itTerm->second.ftc -= p->ft;

            vector<pair<int,InfTermDoc>>& v = itTerm->second.l_docs;
            for (vector<pair<int,InfTermDoc>>::iterator vi = v.begin(); vi != v.end(); ++vi) {
                if (vi->first == idABorrar) {
                    v.erase(vi);
                    break;
                }
            }

            if (v.empty()) {
                itTerm = indice.erase(itTerm);
                continue;
            }
        }
        ++itTerm;
    }

    indiceDocs.erase(itDoc);
    informacionColeccionDocs.numTotalPalDiferentes = static_cast<int>(indice.size());
    return true;
}

bool IndexadorHash::Indexar(const string& ficheroDocumentos) {
    FILE* lista = fopen(ficheroDocumentos.c_str(), "rb");
    if (!lista) return false;

    stemmerPorter sp;

    const bool guardarPos = almacenarPosTerm;
    const int  tipoStem= tipoStemmer;

    const int TAM_BLOQUE = 65536;
    char bufLectura[TAM_BLOQUE];
    string nomFich;
    string resto;
    nomFich.reserve(512);
    resto.reserve(512);

    vector<string> tokens;
    string palabraStem;
    palabraStem.reserve(64);

    string contenido;
    contenido.reserve(100000);

    auto procesarFichero = [&](const string& nomFich) {
        if (nomFich.empty()) {
            return;
        }

        struct stat st;
        if (stat(nomFich.c_str(), &st) != 0) {
            return;
        }

        int idAsignado;
        unordered_map<string, InfDoc>::iterator itExistente = indiceDocs.find(nomFich);
        if (itExistente != indiceDocs.end()) {
            if (st.st_mtime > itExistente->second.fechaModificacion) {
                idAsignado = itExistente->second.idDoc;
                BorraDoc(nomFich);
            }
            else {
                return;
            }
        }
        else {
            idAsignado = ++contadorID;
        }

        FILE* fDoc = fopen(nomFich.c_str(), "rb");
        if (!fDoc) {
            return;
        }

        size_t size = static_cast<size_t>(st.st_size);
        if (size == 0) {
            fclose(fDoc);
            return;
        }

        contenido.resize(size);
        size_t totalLeido = fread(&contenido[0], 1, size, fDoc);
        fclose(fDoc);
        contenido.resize(totalLeido);

        InfDoc infoD;
        infoD.idDoc= idAsignado;
        infoD.tamBytes= st.st_size;
        infoD.fechaModificacion=st.st_mtime;

        int pos=0;

        tok.TokenizarRapido(contenido, tokens);

        for (const string& palabra_orig : tokens) {
            infoD.numPal++;

            if (stopWords.find(palabra_orig) != stopWords.end()) {
                pos++;
                continue;
            }

            infoD.numPalSinParada++;

            const string* pPalabra = &palabra_orig;
            if (tipoStem != 0) {
                palabraStem = palabra_orig;
                sp.stemmer(palabraStem, tipoStem);
                pPalabra = &palabraStem;
            }

            InformacionTermino& infT = indice[*pPalabra];

            if (infT.l_docs.empty() || infT.l_docs.back().first != idAsignado) {
                infT.l_docs.emplace_back(idAsignado, InfTermDoc());
                infoD.numPalDiferentes++;
            }

            InfTermDoc& itd = infT.l_docs.back().second;
            itd.ft++;
            infT.ftc++;
            if (guardarPos) {
                itd.posTerm.push_back(pos);
            }
            pos++;
        }

        indiceDocs[nomFich] = infoD;
        informacionColeccionDocs.numDocs++;
        informacionColeccionDocs.numTotalPal+= infoD.numPal;
        informacionColeccionDocs.numTotalPalSinParada += infoD.numPalSinParada;
        informacionColeccionDocs.tamBytes+= infoD.tamBytes;
    };

    size_t bytesLeidosMain;
    while ((bytesLeidosMain = fread(bufLectura, 1, TAM_BLOQUE, lista)) > 0) {
        size_t pos = 0;
        while (pos < bytesLeidosMain) {
            size_t start = pos;

            while (pos < bytesLeidosMain && bufLectura[pos] != '\n') {
                pos++;
            }

            if (pos < bytesLeidosMain) {
                if (!resto.empty()) {
                    resto.append(bufLectura + start, pos - start);
                    if (!resto.empty() && resto.back() == '\r') {
                        resto.pop_back();
                    }
                    procesarFichero(resto);
                    resto.clear();
                }
                else {
                    size_t len = pos - start;
                    if (len > 0 && bufLectura[pos - 1] == '\r') {
                        len--;
                    }
                    nomFich.assign(bufLectura + start, len);
                    procesarFichero(nomFich);
                }
                pos++;
            }
            else {
                resto.append(bufLectura + start, pos - start);
            }
        }
    }

    if (!resto.empty()) {
        if (resto.back() == '\r') {
            resto.pop_back();
        }
        procesarFichero(resto);
    }

    fclose(lista);
    informacionColeccionDocs.numTotalPalDiferentes = indice.size();
    return true;
}

bool IndexadorHash::IndexarDirectorio(const string& dirAIndexar) {
    DIR* dp = opendir(dirAIndexar.c_str());
    if (!dp) {
        return false;
    }

    bool res = true;
    vector<string> ficheros;
    struct dirent* dirp;

    while ((dirp = readdir(dp)) != NULL) {
        string name = string(dirp->d_name);
        if (name != "." && name != "..") {
            string ruta = dirAIndexar + "/" + name;
            struct stat st;
            if (stat(ruta.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    if (!IndexarDirectorio(ruta)) {
                        res = false;
                    }
                }
                else {
                    ficheros.push_back(ruta);
                }
            }
        }
    }
    closedir(dp);

    if (ficheros.empty()) {
        return res;
    }

    sort(ficheros.begin(), ficheros.end());

    stemmerPorter sp;
    list<string> tokens;

    for (vector<string>::iterator itFich = ficheros.begin(); itFich != ficheros.end(); ++itFich) {

        const string& nomFich = *itFich;

        struct stat st;
        if (stat(nomFich.c_str(), &st) != 0) {
            continue;
        }

        int idAsignado;
        unordered_map<string, InfDoc>::iterator itExistente = indiceDocs.find(nomFich);
        if (itExistente != indiceDocs.end()) {
            if (st.st_mtime > itExistente->second.fechaModificacion) {
                idAsignado = itExistente->second.idDoc;
                BorraDoc(nomFich);
            }
            else {
                continue;
            }
        }
        else {
            idAsignado = ++contadorID;
        }

        FILE* fDoc = fopen(nomFich.c_str(), "rb");
        if (!fDoc) {
            continue;
        }

        size_t size = static_cast<size_t>(st.st_size);
        if (size == 0) {
            fclose(fDoc);
            continue;
        }

        string contenido;
        contenido.resize(size);

        char bufDoc[65536];
        size_t totalLeido = 0;
        size_t bytesLeidos;
        while ((bytesLeidos = fread(bufDoc, 1, sizeof(bufDoc), fDoc)) > 0) {
            for (size_t i = 0; i < bytesLeidos; i++){
                if (bufDoc[i] == '\n' || bufDoc[i] == '\r') {
                    bufDoc[i] = ' ';
                }
            }
            memcpy(&contenido[totalLeido], bufDoc, bytesLeidos);
            totalLeido += bytesLeidos;
        }
        fclose(fDoc);
        contenido.resize(totalLeido);
        tok.Tokenizar(contenido, tokens);

        InfDoc infoD;
        infoD.idDoc= idAsignado;
        infoD.tamBytes= st.st_size;
        infoD.fechaModificacion=st.st_mtime;

        int pos = 0;
        for (list<string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
            if (it->empty()) {
                continue;
            }
            infoD.numPal++;

            if (stopWords.find(*it) != stopWords.end()) {
                pos++;
                continue;
            }

            infoD.numPalSinParada++;

            const string* pTermino = &(*it);
            string terminoStemmed;
            if (tipoStemmer != 0) {
                terminoStemmed = *it;
                sp.stemmer(terminoStemmed, tipoStemmer);
                pTermino = &terminoStemmed;
            }

            auto itMap = indice.find(*pTermino);
            if (itMap == indice.end()){
                itMap = indice.emplace(*pTermino, InformacionTermino()).first;
            }

            InformacionTermino& infT = itMap->second;

            if (infT.l_docs.empty() || infT.l_docs.back().first != idAsignado) {
                infT.l_docs.push_back(make_pair(idAsignado, InfTermDoc()));
                infoD.numPalDiferentes++;
            }

            InfTermDoc& itd = infT.l_docs.back().second;
            itd.ft++;
            infT.ftc++;

            if (almacenarPosTerm){
                itd.posTerm.push_back(pos);
            }
            pos++;
        }

        indiceDocs[nomFich] = infoD;
        informacionColeccionDocs.numDocs++;
        informacionColeccionDocs.numTotalPal+= infoD.numPal;
        informacionColeccionDocs.numTotalPalSinParada += infoD.numPalSinParada;
        informacionColeccionDocs.tamBytes+= infoD.tamBytes;
    }

    informacionColeccionDocs.numTotalPalDiferentes = indice.size();
    return res;
}

bool IndexadorHash::GuardarIndexacion() const {
    string dir;
    if (directorioIndice.empty()) {
        dir = ".";
    }
    else {
        dir = directorioIndice;
    }
    mkdir(dir.c_str(), 0755);

    ofstream f((dir + "/indexacion.dat").c_str());
    if (!f) {
        return false;
    }

    f << ficheroStopWords << "\n";
    f << tok.DelimitadoresPalabra() << "\n";
    f << const_cast<Tokenizador&>(tok).CasosEspeciales() << "\n";
    f << const_cast<Tokenizador&>(tok).PasarAminuscSinAcentos() << "\n";
    f << directorioIndice << "\n";
    f << tipoStemmer << "\n";
    f << almacenarPosTerm << "\n";
    f << pregunta << "\n";
    f << contadorID << "\n";

    f << stopWords.size() << "\n";
    for (unordered_set<string>::const_iterator it = stopWords.begin();
         it != stopWords.end(); ++it) {
        f << *it << "\n";
    }

    f << informacionColeccionDocs.numDocs << "\n"
      << informacionColeccionDocs.numTotalPal << "\n"
      << informacionColeccionDocs.numTotalPalSinParada << "\n"
      << informacionColeccionDocs.numTotalPalDiferentes << "\n"
      << informacionColeccionDocs.tamBytes << "\n";

    f << indiceDocs.size() << "\n";
    for (unordered_map<string,InfDoc>::const_iterator it = indiceDocs.begin(); it != indiceDocs.end(); ++it) {
        f << it->first << "\n"
          << it->second.idDoc << "\n"
          << it->second.numPal << "\n"
          << it->second.numPalSinParada << "\n"
          << it->second.numPalDiferentes << "\n"
          << it->second.tamBytes << "\n"
          << it->second.fechaModificacion << "\n";
    }

    f << indice.size() << "\n";
    for (unordered_map<string,InformacionTermino>::const_iterator it = indice.begin(); it != indice.end(); ++it) {
        f << it->first << "\n"
          << it->second.ftc << "\n"
          << it->second.l_docs.size() << "\n";
        for (vector<pair<int,InfTermDoc>>::const_iterator itD = it->second.l_docs.begin(); itD != it->second.l_docs.end(); ++itD) {
            f << itD->first << "\n"
              << itD->second.ft << "\n"
              << itD->second.posTerm.size() << "\n";
            for (vector<int>::const_iterator itP = itD->second.posTerm.begin(); itP != itD->second.posTerm.end(); ++itP) {
                f << *itP << "\n";
            }
        }
    }

    f << indicePregunta.size() << "\n";
    for (unordered_map<string,InformacionTerminoPregunta>::const_iterator it = indicePregunta.begin(); it != indicePregunta.end(); ++it) {
        f << it->first << "\n"
          << it->second.ft << "\n"
          << it->second.posTerm.size() << "\n";
        for (vector<int>::const_iterator itP = it->second.posTerm.begin(); itP != it->second.posTerm.end(); ++itP) {
            f << *itP << "\n";
        }
    }

    f << infPregunta.numTotalPal << "\n"
      << infPregunta.numTotalPalSinParada << "\n"
      << infPregunta.numTotalPalDiferentes << "\n";

    return true;
}

bool IndexadorHash::RecuperarIndexacion(const string& directorioIndexacion) {
    VaciarIndiceDocs();
    VaciarIndicePreg();
    stopWords.clear();

    string dir;
    if (directorioIndexacion.empty()) {
        dir = ".";
    }
    else {
        dir = directorioIndexacion;
    }

    ifstream f((dir + "/indexacion.dat").c_str());
    if (!f) {
        return false;
    }

    getline(f, ficheroStopWords);
    string delims;
    getline(f, delims);
    bool casosEsp, minusc;
    f >> casosEsp >> minusc;
    f.ignore();
    getline(f, directorioIndice);
    f >> tipoStemmer >> almacenarPosTerm;
    f.ignore();
    getline(f, pregunta);
    f >> contadorID;
    f.ignore();

    tok.DelimitadoresPalabra(delims);
    tok.CasosEspeciales(casosEsp);
    tok.PasarAminuscSinAcentos(minusc);

    int nSW; f >> nSW; f.ignore();
    stopWords.reserve(nSW * 2);
    for (int i = 0; i < nSW; i++) {
        string sw;
        getline(f, sw);
        stopWords.insert(sw);
    }

    f >> informacionColeccionDocs.numDocs
      >> informacionColeccionDocs.numTotalPal
      >> informacionColeccionDocs.numTotalPalSinParada
      >> informacionColeccionDocs.numTotalPalDiferentes
      >> informacionColeccionDocs.tamBytes;
    f.ignore();

    int nDocs; f >> nDocs; f.ignore();
    indiceDocs.reserve(nDocs * 2);
    for (int i = 0; i < nDocs; i++) {
        string nomDoc;
        getline(f, nomDoc);
        InfDoc d;
        f >> d.idDoc >> d.numPal >> d.numPalSinParada
          >> d.numPalDiferentes >> d.tamBytes >> d.fechaModificacion;
        f.ignore();
        indiceDocs[nomDoc] = d;
    }

    int nTerms; f >> nTerms; f.ignore();
    indice.reserve(nTerms * 2);
    for (int i = 0; i < nTerms; i++) {
        string term;
        getline(f, term);
        InformacionTermino it;
        f >> it.ftc;
        int nDocsT; f >> nDocsT; f.ignore();
        it.l_docs.reserve(nDocsT);
        for (int j = 0; j < nDocsT; j++) {
            int idDoc;
            f >> idDoc;
            InfTermDoc itd;
            f >> itd.ft;
            int nPos; f >> nPos; f.ignore();
            for (int k = 0; k < nPos; k++) {
                int pos; f >> pos;
                itd.posTerm.push_back(pos);
            }
            if (nPos > 0) {
                f.ignore();
            }
            it.l_docs.push_back(make_pair(idDoc, itd));
        }
        indice[term] = it;
    }

    int nPreg; f >> nPreg; f.ignore();
    for (int i = 0; i < nPreg; i++) {
        string term;
        getline(f, term);
        InformacionTerminoPregunta itp;
        f >> itp.ft;
        int nPos; f >> nPos; f.ignore();
        for (int k = 0; k < nPos; k++) {
            int pos; f >> pos;
            itp.posTerm.push_back(pos);
        }
        if (nPos > 0) {
            f.ignore();
        }
        indicePregunta[term] = itp;
    }

    f >> infPregunta.numTotalPal
      >> infPregunta.numTotalPalSinParada
      >> infPregunta.numTotalPalDiferentes;

    return true;
}