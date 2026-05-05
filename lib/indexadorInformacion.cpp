#include "indexadorInformacion.h"

using namespace std;

//Clase InformaciónTermino
InformacionTermino::InformacionTermino (const InformacionTermino &p){
    this->ftc=p.ftc;
    this->l_docs=p.l_docs;
}

InformacionTermino::InformacionTermino() : ftc(0) {
}

InformacionTermino:: ~InformacionTermino (){
    this->ftc=0;
}

InformacionTermino & InformacionTermino::operator= (const InformacionTermino &p){
    if (this != &p) {
        this->ftc = p.ftc;
        this->l_docs = p.l_docs;
    }
    return *this;
}

InfTermDoc& InformacionTermino::operator[](int idDoc) {
    for (vector<pair<int,InfTermDoc>>::iterator it = l_docs.begin(); it != l_docs.end(); ++it) {
        if (it->first == idDoc) {
            return it->second;
        }
    }
    l_docs.push_back(make_pair(idDoc, InfTermDoc()));
    return l_docs.back().second;
}

InfTermDoc* InformacionTermino::find(int idDoc) {
    for (vector<pair<int,InfTermDoc>>::iterator it = l_docs.begin(); it != l_docs.end(); ++it) {
        if (it->first == idDoc) {
            return &it->second;
        }
    }
    return 0;
}

const InfTermDoc* InformacionTermino::find(int idDoc) const {
    for (vector<pair<int,InfTermDoc>>::const_iterator it = l_docs.begin(); it != l_docs.end(); ++it) {
        if (it->first == idDoc) {
            return &it->second;
        }
    }
    return 0;
}

ostream& operator<<(ostream& s, const InformacionTermino& p) {
    s << "Frecuencia total: " << p.ftc << "\tfd: " << p.l_docs.size();
    for (vector<pair<int,InfTermDoc>>::const_iterator it = p.l_docs.begin(); it != p.l_docs.end(); ++it) {
        s << "\tId.Doc: " << it->first << "\t" << it->second;
    }
    return s;
}

//Clase InfTermDoc
InfTermDoc::InfTermDoc() : ft(0) {
}

InfTermDoc::InfTermDoc(const InfTermDoc &p) {
    this->ft = p.ft;
    this->posTerm = p.posTerm;
}

InfTermDoc::~InfTermDoc() {
    this->ft=0;
}

InfTermDoc& InfTermDoc::operator=(const InfTermDoc &p) {
    if (this != &p) {
        this->ft = p.ft;
        this->posTerm = p.posTerm;
    }
    return *this;
}

ostream& operator<<(ostream& s, const InfTermDoc& p) {
    s << "ft: " << p.ft;
    for (vector<int>::const_iterator it = p.posTerm.begin(); it != p.posTerm.end(); ++it) {
        s << "\t" << *it;
    }
    return s;
}

//Clase InfDoc
InfDoc::InfDoc() : idDoc(0), numPal(0), numPalSinParada(0),
                   numPalDiferentes(0), tamBytes(0), fechaModificacion(0) {
}

InfDoc::InfDoc(const InfDoc &p) {
    idDoc = p.idDoc;
    numPal = p.numPal;
    numPalSinParada = p.numPalSinParada;
    numPalDiferentes = p.numPalDiferentes;
    tamBytes = p.tamBytes;
    fechaModificacion = p.fechaModificacion;
}

InfDoc::~InfDoc() {
}

InfDoc& InfDoc::operator=(const InfDoc &p) {
    if (this != &p) {
        idDoc = p.idDoc;
        numPal = p.numPal;
        numPalSinParada = p.numPalSinParada;
        numPalDiferentes = p.numPalDiferentes;
        tamBytes = p.tamBytes;
        fechaModificacion = p.fechaModificacion;
    }
    return *this;
}

ostream& operator<<(ostream& s, const InfDoc& p) {
    s << "idDoc: " << p.idDoc
      << "\tnumPal: " << p.numPal
      << "\tnumPalSinParada: " << p.numPalSinParada
      << "\tnumPalDiferentes: " << p.numPalDiferentes
      << "\ttamBytes: " << p.tamBytes;
    return s;
}

//Clase InfColeccionDocs
InfColeccionDocs::InfColeccionDocs() : numDocs(0), numTotalPal(0),
                                       numTotalPalSinParada(0),
                                       numTotalPalDiferentes(0), tamBytes(0) {
}

InfColeccionDocs::InfColeccionDocs(const InfColeccionDocs &p) {
    numDocs = p.numDocs;
    numTotalPal = p.numTotalPal;
    numTotalPalSinParada = p.numTotalPalSinParada;
    numTotalPalDiferentes = p.numTotalPalDiferentes;
    tamBytes = p.tamBytes;
}

InfColeccionDocs::~InfColeccionDocs() {
}

InfColeccionDocs& InfColeccionDocs::operator=(const InfColeccionDocs &p) {
    if (this != &p) {
        numDocs = p.numDocs;
        numTotalPal = p.numTotalPal;
        numTotalPalSinParada = p.numTotalPalSinParada;
        numTotalPalDiferentes = p.numTotalPalDiferentes;
        tamBytes = p.tamBytes;
    }
    return *this;
}

ostream& operator<<(ostream& s, const InfColeccionDocs& p) {
    s << "numDocs: " << p.numDocs
      << "\tnumTotalPal: " << p.numTotalPal
      << "\tnumTotalPalSinParada: " << p.numTotalPalSinParada
      << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes
      << "\ttamBytes: " << p.tamBytes;
    return s;
}

//Clase InformacionTerminoPregunta
InformacionTerminoPregunta::InformacionTerminoPregunta() : ft(0) {
}

InformacionTerminoPregunta::InformacionTerminoPregunta(const InformacionTerminoPregunta &p) {
    ft = p.ft;
    posTerm = p.posTerm;
}

InformacionTerminoPregunta::~InformacionTerminoPregunta() {
}

InformacionTerminoPregunta& InformacionTerminoPregunta::operator=(const InformacionTerminoPregunta &p) {
    if (this != &p) {
        ft = p.ft;
        posTerm = p.posTerm;
    }
    return *this;
}

ostream& operator<<(ostream& s, const InformacionTerminoPregunta& p) {
    s << "ft: " << p.ft;
    for (vector<int>::const_iterator it = p.posTerm.begin(); it != p.posTerm.end(); ++it) {
        s << "\t" << *it;
    }
    return s;
}

//Clase InformacionPregunta
InformacionPregunta::InformacionPregunta() : numTotalPal(0),
                                             numTotalPalSinParada(0),
                                             numTotalPalDiferentes(0) {
}

InformacionPregunta::InformacionPregunta(const InformacionPregunta &p) {
    numTotalPal = p.numTotalPal;
    numTotalPalSinParada = p.numTotalPalSinParada;
    numTotalPalDiferentes = p.numTotalPalDiferentes;
}

InformacionPregunta::~InformacionPregunta() {
}

InformacionPregunta& InformacionPregunta::operator=(const InformacionPregunta &p) {
    if (this != &p) {
        numTotalPal = p.numTotalPal;
        numTotalPalSinParada = p.numTotalPalSinParada;
        numTotalPalDiferentes = p.numTotalPalDiferentes;
    }
    return *this;
}

ostream& operator<<(ostream& s, const InformacionPregunta& p) {
    s << "numTotalPal: " << p.numTotalPal
      << "\tnumTotalPalSinParada: " << p.numTotalPalSinParada
      << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes;
    return s;
}

