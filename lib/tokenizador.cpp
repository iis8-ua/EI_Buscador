#include "tokenizador.h"

Tokenizador::Tokenizador (){
    DelimitadoresPalabra(",;:.-/+*\\ '\"{}[]()<>?!??&#=\t@"); //para que se rellene tambien el array
    casosEspeciales=true;
    pasarAminuscSinAcentos=false;
}

Tokenizador::Tokenizador (const Tokenizador& t){
    //para que se copie tambien el array
    DelimitadoresPalabra(t.delimiters);
    this->casosEspeciales=t.casosEspeciales;
    this->pasarAminuscSinAcentos=t.pasarAminuscSinAcentos;
}

Tokenizador::Tokenizador (const string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos){
    this->casosEspeciales=kcasosEspeciales;
    this->pasarAminuscSinAcentos=minuscSinAcentos;
    DelimitadoresPalabra(delimitadoresPalabra);
}

Tokenizador::~Tokenizador (){
    delimiters="";
}

Tokenizador& Tokenizador::operator= (const Tokenizador& t){
    if(this != &t){
        this->delimiters=t.delimiters;
        this->casosEspeciales=t.casosEspeciales;
        this->pasarAminuscSinAcentos=t.pasarAminuscSinAcentos;
    }
    return *this;
}

void Tokenizador::Tokenizar (const string& str, list<string>& tokens) const{
    //versi?n simple vista en teoria, esta no se toca y no se mide la eficiencia
    papelera.splice(papelera.end(), tokens);
    if(str.empty()){
        return;
    }
    string strNormalizado;
    //se usa un puntero para que al normalizar se apunta a la copia y asi si es falso pasarAMinusculas se apunta al original y no se copia de nuevo
    const string* punteroStr = &str;

    if (pasarAminuscSinAcentos) {
        strNormalizado = Normalizar(str);
        punteroStr = &strNormalizado;
    }

    if(casosEspeciales){
        TokenizarCasosEspeciales(*punteroStr, tokens);
    }
    else{
        string::size_type lastPos = punteroStr->find_first_not_of(delimiters, 0);
        string::size_type pos = punteroStr->find_first_of(delimiters, lastPos);

       while (string::npos != pos || string::npos != lastPos) {
           if (!papelera.empty()) {
                papelera.front().assign(*punteroStr, lastPos, pos - lastPos);
                tokens.splice(tokens.end(), papelera, papelera.begin());
           }
           else {
               tokens.push_back(string(*punteroStr, lastPos, pos - lastPos));
           }

           lastPos = punteroStr->find_first_not_of(delimiters, pos);
           pos = punteroStr->find_first_of(delimiters, lastPos);
       }
    }

}

bool Tokenizador::Tokenizar (const string& i, const string& f) const {
    FILE* entrada = fopen(i.c_str(), "rb");
    if (!entrada) {
        cerr << "ERROR: No existe el archivo: " << i << '\n';
        return false;
    }

    FILE* salida = fopen(f.c_str(), "wb");
    if (!salida) {
        cerr << "ERROR: No se pudo crear el archivo: " << f << '\n';
        fclose(entrada);
        return false;
    }

    //buffer de escritura en pila
    char bufferEscritura[131072];
    setvbuf(salida, bufferEscritura, _IOFBF, sizeof(bufferEscritura));

    const int TAM_BLOQUE = 65536;
    char bufferLectura[TAM_BLOQUE];

    string linea;
    linea.reserve(4096);
    string resto;
    resto.reserve(4096);

    list<string> tokensLocales;
    papelera.splice(papelera.end(), tokensLocales);

    size_t bytesLeidos;

    while ((bytesLeidos = fread(bufferLectura, 1, TAM_BLOQUE, entrada)) > 0) {
        size_t pos = 0;

        while (pos < bytesLeidos) {
            size_t start = pos;

            while (pos < bytesLeidos && bufferLectura[pos] != '\n') {
                pos++;
            }

            if (pos < bytesLeidos) {
                if (!resto.empty()) {
                    resto.append(bufferLectura + start, pos - start);
                    if (!resto.empty() && resto[resto.length() - 1] == '\r') {
                        resto.erase(resto.length() - 1);
                    }
                    Tokenizar(resto, tokensLocales);
                    resto.clear();
                }
                else {
                    size_t len = pos - start;
                    if (len > 0 && bufferLectura[pos - 1] == '\r') {
                        len--;
                    }
                    linea.assign(bufferLectura + start, len);
                    Tokenizar(linea, tokensLocales);
                }

                list<string>::iterator it;
                for (it = tokensLocales.begin(); it != tokensLocales.end(); ++it) {
                    fwrite(it->data(), 1, it->length(), salida);
                    fputc('\n', salida);
                }

                papelera.splice(papelera.end(), tokensLocales);
                pos++;
            }
            else {
                resto.append(bufferLectura + start, pos - start);
            }
        }
    }

    if (!resto.empty()) {
        if (resto[resto.length() - 1] == '\r') {
            resto.erase(resto.length() - 1);
        }
        Tokenizar(resto, tokensLocales);

        list<string>::iterator it;
        for (it = tokensLocales.begin(); it != tokensLocales.end(); ++it) {
            fwrite(it->data(), 1, it->length(), salida);
            fputc('\n', salida);
        }
        papelera.splice(papelera.end(), tokensLocales);
    }

    fclose(entrada);
    fclose(salida);
    return true;
}

bool Tokenizador::Tokenizar (const string & i) const{
    string f = i + ".tk";
    return Tokenizar(i, f);
}

bool Tokenizador::TokenizarListaFicheros (const string& i) const{
    ifstream lista(i.c_str());
    if (!lista) {
        cerr << "ERROR: No existe el archivo de lista: " << i << '\n';
        return false;
    }

    string nombreFichero;
    //prereservamos memoria para este nombre
    nombreFichero.reserve(256);
    bool todoCorrecto = true;
    //se saca fuera para que no se haga en cada iteracion del bucle
    struct stat dir;

    while (getline(lista, nombreFichero)) {
        if (nombreFichero.empty()){
            continue;
        }

        //ignorar si es un directorio
        if(stat(nombreFichero.c_str(), &dir)==0){
            if(S_ISDIR(dir.st_mode)){
                cerr << "ERROR: El elemento de la lista es un directorio: " << nombreFichero << '\n';
                todoCorrecto = false;
                continue;
            }
        }

        //se llama a la que va uno por uno (la que a?ade la extension tk, que se encarga de llamar a la otra) para que vaya tokenizando fichero por fichero
        if (!Tokenizar(nombreFichero)) {
            todoCorrecto = false;
        }
    }

    lista.close();
    return todoCorrecto;
}

bool Tokenizador::TokenizarDirectorio (const string& i) const{
    struct stat dir;
    int err=stat(i.c_str(), &dir);

    //ISDIR comprueba si es un directorio
    if(err==-1 || !S_ISDIR(dir.st_mode)){
        cerr << "ERROR: No existe el directorio o no es un directorio: " << i << '\n';
        return false;
    }
    else {
        // Hago una lista en un fichero con find>fich, es una lista con todos los archivos y la guarda en el fichero temporal
        //con follow aseguras que sigue enlaces simbolicos
        string cmd="find "+i+" -follow |sort > .lista_fich";
        system(cmd.c_str());
        return TokenizarListaFicheros(".lista_fich");
    }
}

void Tokenizador::DelimitadoresPalabra(const string& nuevoDelimiters){
    delimiters="";
    //Se inicializa el array de delimitadores a false
    for(int i=0; i<256; ++i){
        es_delim[i]=false;
    }

    //los blancos son delimitadores siempre
    es_delim[(unsigned char)' '] = true;
    es_delim[(unsigned char)'\n'] = true;
    es_delim[(unsigned char)'\r'] = true;

    for(int i=0; i<nuevoDelimiters.length(); ++i){
        char c=nuevoDelimiters[i];
        if(delimiters.find(c)==string::npos){
            delimiters +=c;
            //se guarda tambien en el array
            es_delim[(unsigned char)c]=true;
        }
    }
}

void Tokenizador::AnyadirDelimitadoresPalabra(const string& nuevoDelimiters){
    for (int i=0; i<nuevoDelimiters.length(); ++i){
        char c=nuevoDelimiters[i];
        if(delimiters.find(c) == string::npos){
            delimiters +=c;
            //actualizo el array
            es_delim[(unsigned char)c]=true;
        }
    }
}

string Tokenizador::DelimitadoresPalabra() const{
    return delimiters;
}

void Tokenizador::CasosEspeciales (const bool& nuevoCasosEspeciales){
    this->casosEspeciales=nuevoCasosEspeciales;
}

bool Tokenizador::CasosEspeciales (){
    return casosEspeciales;
}

void Tokenizador::PasarAminuscSinAcentos (const bool& nuevoPasarAminuscSinAcentos){
    this->pasarAminuscSinAcentos=nuevoPasarAminuscSinAcentos;
}

bool Tokenizador::PasarAminuscSinAcentos (){
    return pasarAminuscSinAcentos;
}

ostream& operator<<(ostream& os, const Tokenizador& t){
     os<< "DELIMITADORES: " << t.delimiters << " TRATA CASOS ESPECIALES: " << t.casosEspeciales << " PASAR A MINUSCULAS Y SIN ACENTOS: " << t.pasarAminuscSinAcentos;
     return os;
}

void Tokenizador::TokenizarCasosEspeciales(const string& str, list<string>& tokens) const{
    //lo que hace es punteros crudos para evitar str.operator[]
    const char* strData = str.data();
    int len= str.length();

    //buffer estatico en lugar de std::string, lo que hace O3
    char tokenBuf[2048];
    int tokenLen = 0;

    //para evitar hacer bucles anidados y que me hacian tener esa complejidad cuadratica,
    //creo una maquina de estados para memorizar el estado

    //para las @ solo habra que comprobar que es == 0, si es asi la a?ado al token y le sumo uno a esta variable, si se encuantra otra mas delante esta variable va a ser 1 y se rechaza (delimitador normal)
    int arrobas_en_token=0;
    int fin_palabra_email = -1;
    bool es_email_valido = false;
    bool token_tiene_delimitador = false;

    //Para los numeros se hace lo mismo, este bool se actualiza letra a letra, donde si llevo un numero y viene un . seguido de un digito lo a?ado y sigo,
    //pero si vienen letras despues de un punto, se pone esto a false y se deja de considerar un numero
    bool token_es_numero=true;
    int fin_palabra_num = -1;
    bool resto_es_numero = true;

    //Para poder controlar que es una URL
    bool es_url=false;

    for(int i=0; i<len; ++i){
        //aqui tenemos el caracter del string
        char c=strData[i];
        //se pasa a unsigned para trabajar y buscar el delimitador en el array estatico
        unsigned char uc = (unsigned char)c;
        //asi la complejidad es O(1) en vez de delimiters.find
        bool encontrado =es_delim[uc];

        if(!encontrado){
            //caso en el que no estamos en un delimitador por lo que se a?ade el caracter para formar el token
            //asignacion directa en memoria
            tokenBuf[tokenLen++] = c;
            //se tiene que ir actualizando la maquina de estados en cada iteracion
            if(c=='@'){
                arrobas_en_token++;
            }
            if(!isdigit(uc)){
                token_es_numero=false;
            }
        }
        else{
            //si que es un delimitador, por lo que hay que ver como dice en el enunciado si hay que meterlo o no en el token
            //porque nos dice que el orden de los ifs es importante por lo que un delimitador puede que sea parte del token teniendo
            //en cuenta esos ifs

            //este booleano es el que va a hacer que se mantega o no el delimitador en el token
            bool saltar=false;
            //esta lo que va a tener es el siguiente caracter del string para luego poder usarlo en los diferentes casos especiales
            char siguienteC;
            if(i+1<len){
                siguienteC=str[i+1];
            }
            else{
                //es como si hubiera un blanco ya que se acaba el string, se hace asi para cortar
                siguienteC=0;
            }
            //tambien para el array de busqueda
            unsigned char usiguienteC =(unsigned char) siguienteC;

            //CASO URLs --> Delimitadores especiales: ?_:/.?&-=#@?
            //se activa comience por SOLO los indicadores de URL ?http:? o ?https:? o ?ftp:? (en min?sculas)
            // seguido por una secuencia de caracteres (incluidos ?_:/.?&-=#@) sin ning?n blanco por medio.
            //Finalizar? cuando se detecte un delimitador (excepto ?_:/.?&-=#@?) o un blanco (
            if(!saltar && tokenLen>0){
                //si aun no es una URL y viene los : a continuaci?n
                if(!es_url && c==':'){
                    //evto crear una nueva variable string como hacia antes y evito ese for cada vez que ve dos puntos
                    //ahora voy accediando caracter a caracter dependiendo de la longitud y veo que es lo que pide
                    bool inicio_url=false;
                    if (tokenLen == 3) {
                        if (tokenBuf[0] == 'f' && tokenBuf[1] == 't' && tokenBuf[2] == 'p'){
                            inicio_url = true;
                        }
                    }
                    else if (tokenLen== 4) {
                        if (tokenBuf[0] == 'h' && tokenBuf[1] == 't' && tokenBuf[2]== 't' && tokenBuf[3] == 'p'){
                            inicio_url = true;
                        }
                    }
                    else if (tokenLen== 5) {
                        if (tokenBuf[0] == 'h' && tokenBuf[1] == 't' && tokenBuf[2] == 't' && tokenBuf[3] == 'p' && tokenBuf[4] == 's') {
                            inicio_url = true;
                        }
                    }

                    //hay que comprobar que si c es :, hay que ver que lo que tenemos en el token es solo http, https o ftp
                    if(inicio_url){
                        //aqui vuelvo a no usar el find que recorre el string por un switch con los caracteres delimitadores del find
                        bool es_delim_url = false;
                        switch (siguienteC) {
                            case '_': case ':': case '/': case '.': case '?':
                            case '&': case '-': case '=': case '#': case '@':
                                es_delim_url = true;
                                break;
                        }
                        //a?adimos los : si el siguiente de estos dos puntos no es blanco o no esta en los permitidos de la lista
                        bool validoURL = (isalnum(usiguienteC) || es_delim_url);
                        if (siguienteC != 0 && validoURL) {
                            es_url = true;
                            saltar = true;
                        }
                    }
                }
                else if(es_url){
                    //se vuelve a hacer lo mismo para evitar el find
                    bool es_delim_url = false;
                    switch (c) {
                        case '_': case ':': case '/': case '.': case '?':
                        case '&': case '-': case '=': case '#': case '@':
                            es_delim_url = true;
                            break;
                    }

                    if (es_delim_url) {
                        saltar = true;
                    }
                    else {
                        es_url = false;
                    }
                }
            }

            //CASO NUMEROS --> Delimitadores especiales: ?.,?
            //aparecen al principio del t?rmino o por el medio y solo acompa?ado por numeros (no cientificos)
            if(!saltar && (c=='.' || c==',')){
                //hay que ver que sea numero el token y que no este vacio
                bool numero= (token_es_numero && tokenLen>0);

                //antes teniamos un bucle que miraba lo que ya teniamos generando complejidad cuadratica, por lo que se elimina y se hace una vez por palabra
                if(i>=fin_palabra_num){
                    resto_es_numero=true;
                    fin_palabra_num=i+1;

                    //ahora hay que ver el resto hasta el final de la palabra ya que se puede confundir con un acronimo
                    while(fin_palabra_num<len){
                        char sig=str[fin_palabra_num];
                        bool blancoNumero = (sig == ' ' || sig == '\n' || sig == '\r');
                        bool delimitadorNumero = es_delim[(unsigned char)sig] || blancoNumero;

                         //si tenemos un delimitador que no sea , o . ya no puede ser un numero
                        if(delimitadorNumero && sig != '.' && sig != ','){
                            break;
                        }

                        //se encuentra algo que no es un digito ni un separador, por lo que ya no es un numero
                        if(!isdigit((unsigned char)sig) && !delimitadorNumero){
                            //se pone a false para que llegue al final y no se vuelva a escanear esa palabra
                            resto_es_numero=false;
                        }
                        fin_palabra_num++;
                    }
                }

                if(resto_es_numero){
                    //ejemplo 3.5
                    if (numero && isdigit(usiguienteC)) {
                        saltar = true;
                    }
                    //ejemplo .67 -> 0.67
                    else if (tokenLen==0 && isdigit(usiguienteC)){
                        tokenBuf[tokenLen++] = '0';
                        saltar=true;
                        token_es_numero=true;
                    }
                }
            }

            //comun para los casos siguientes
            bool siguienteDelimitador;
            if(siguienteC==0){
                siguienteDelimitador=true;
            }
            else{
                siguienteDelimitador=es_delim[usiguienteC];
            }

            //CASO EMAIL --> Delimitadores especiales: ?@?.
            // detectar el @ por el medio de un t?rmino siendo ?ste delimitador.
            //inicia por un blanco o separador seguido de cualquier car?cter, y contiene un solo ?@? por medio
            //detectar? el final por la presencia de un espacio en blanco
            if(!saltar){
                //solo puede haber un @ en un email por lo que hay que contar ya que si hay dos no se tiene que tratar como un email
                if(c=='@'){
                    //si no esta vacio y es la primera arroba hace que mirar hacia atras tenga complejidad O(1), tampoco tiene que tener delimitadores previos
                    if(arrobas_en_token ==0 && tokenLen > 0 && !token_tiene_delimitador){
                        //ahora se mira hacia delante sin escanearlo muchas veces, haciendo que sea complejidad O(N)
                        if(i>=fin_palabra_email){
                            es_email_valido=true;
                            fin_palabra_email=i+1;
                            int contar_arrobas=1; //se cuenta la actual

                            //ahora hasta el final de la palabra
                            while(fin_palabra_email<len){
                                char sig = strData[fin_palabra_email];
                                //si se encuentra un blanco se para
                                bool blancoArroba=(sig == ' ' || sig == '\n' || sig == '\r');
                                if (blancoArroba){
                                    break;
                                }

                                if(sig=='@'){
                                    contar_arrobas++;
                                    break;
                                }
                                else{
                                    //si se encuentra uno fuerte que no esta permitido se para ya de mirar
                                    bool es_del = es_delim[(unsigned char)sig];
                                    if (es_del && sig != '.' && sig != '-' && sig != '_') {
                                         break;
                                    }
                                }
                                fin_palabra_email++;
                            }

                            //si hay mas de una ya no es un email
                            if(contar_arrobas !=1){
                                es_email_valido=false;
                            }
                        }
                        if(es_email_valido){
                            saltar=true;
                        }
                    }
                }

                //puede contner tanto _ como -  como . hay que controlar eso ya que es solo despues del @ y rodeados de otro no delimitador y sin ningun blanco
                else if (tokenLen > 0){
                    //se hace para evitar el find como en los demas
                    bool es_delim_email=false;
                    switch(c){
                        case '.': case '-': case '_':
                            es_delim_email = true;
                            break;
                    }

                    if(es_delim_email && arrobas_en_token >0){
                        //ahora hay que ver lo que tiene rodeado, es decir el siguiente no puede ser delimitador ni blanco
                        if(!siguienteDelimitador){
                            //si se cumple que el siguiente no es blanco y no es delimitador lo incluimos
                            saltar=true;
                        }
                    }
                }
            }

            //CASO ACRONIMOS --> Delimitadores especiales: ?.?
            //se activa cuando se detecte un punto en medio sin ningun blanco y separados por caracteres distintos no vale otro punto
            //para acabar blanco, delimitador o varios puntos seguidos
            if(!saltar && c== '.' && tokenLen>0){
                //tambien hay que ver que luego haya algo valido
                if(siguienteC != '.' && !siguienteDelimitador){
                    saltar=true;
                }
            }

            //CASO MULTIPALABRAS --> Delimitadores especiales: ?-?.
            //detectar un t?rmino que contenga por el medio el car?cter ?-? (NO rodeado dedelimitadores o espacios en blanco,
            //detectar? el inicio y final de la palabra compuesta cuando aparezca el blanco  o cualquiera de los delimitadores
            if(!saltar && c=='-' && tokenLen>0){
                if(!siguienteDelimitador){
                    saltar=true;
                }
            }

            //logica para a?adir o no a?adir el delimitador al token
            if(saltar){
                //el delimitador aqui se a?ade a la palabra ya que ha entrado en un caso especial
                tokenBuf[tokenLen++] = c;
                //ya tiene un delimitador
                token_tiene_delimitador=true;

                //hay que seguir manteniendo los estados actualizados
                if(c=='@'){
                    arrobas_en_token++;
                }
                if(!isdigit(uc) && c!='.' && c!=','){
                    token_es_numero=false;
                }
            }
            else{
                //lo que se hace es que si no se ha entrado en ningun caso especial
                //ese delimitador actua como lo que hemos hecho hasta ahora, es decir, es el fin de la palabra y la a?adimos al vector para almacenarlas
                if(tokenLen>0){
                //si esta en la papelera se reutiliza
                    if(!papelera.empty()){
                        papelera.front().assign(tokenBuf, tokenLen);
                        tokens.splice(tokens.end(), papelera, papelera.begin());
                    }
                    else{
                        tokens.push_back(string(tokenBuf, tokenLen));
                    }
                    //se reinicia para la siguiente a evaluar, donde ahora es con clear() en vez de ="" para mantener la reserva que he hecho al principio
                    tokenLen = 0;

                    //se reinicia tambien la maquina de estados para la nueva palabra
                    arrobas_en_token = 0;
                    token_es_numero = true;
                    es_url = false;
                    token_tiene_delimitador=false;
                }
            }
        }
    }

    //no se si es necesario y se puede omitir, es por si se queda algo en el token
    if(tokenLen > 0){
        //si ya estan en la papelera se reutilizan los nodos para no llamar siempre a pushback y mejorar los allocs
        if(!papelera.empty()){
            papelera.front().assign(tokenBuf, tokenLen);
            tokens.splice(tokens.end(), papelera, papelera.begin());
        }
        else{
            tokens.push_back(string(tokenBuf, tokenLen));
        }
    }
}

string Tokenizador::Normalizar(const string& str) const {
    static bool tabla_creada = false;
    static unsigned char tabla_norm[256];

    //se crea una tabla estatica de busqueda que se inicializa solo la primera vez que se llama a la funcion en el codigo
    if (!tabla_creada) {
        for (int i = 0; i < 256; ++i) {
            tabla_norm[i] = i;
        }
        for (int i = 'A'; i <= 'Z'; ++i) {
            tabla_norm[i] = tolower(i);
        }
        for (int i = 0xC0; i <= 0xC5; ++i) {
            tabla_norm[i] = 'a';
        }
        for (int i = 0xE0; i <= 0xE5; ++i) {
            tabla_norm[i] = 'a';
        }
        for (int i = 0xC8; i <= 0xCB; ++i) {
            tabla_norm[i] = 'e';
        }
        for (int i = 0xE8; i <= 0xEB; ++i) {
            tabla_norm[i] = 'e';
        }
        for (int i = 0xCC; i <= 0xCF; ++i) {
            tabla_norm[i] = 'i';
        }
        for (int i = 0xEC; i <= 0xEF; ++i){
            tabla_norm[i] = 'i';
        }
        for (int i = 0xD2; i <= 0xD6; ++i){
            tabla_norm[i] = 'o';
        }
        for (int i = 0xF2; i <= 0xF6; ++i){
            tabla_norm[i] = 'o';
        }
        for (int i = 0xD9; i <= 0xDC; ++i){
            tabla_norm[i] = 'u';
        }
        for (int i = 0xF9; i <= 0xFC; ++i){
            tabla_norm[i] = 'u';
        }
        tabla_norm[0xD1] = (char)0xF1; //? -> ?
        tabla_norm[0xC7] = 'c'; //? ->c
        tabla_norm[0xE7] = 'c'; //?->c
        tabla_creada = true;
    }

    string aux = str;
    for (int i = 0; i < aux.length(); ++i) {
        //antes teniamos los 8 ifs ahora es solo acceder a la tabla esa en una complejidad de O(1)
        aux[i] = tabla_norm[(unsigned char)aux[i]];
    }
    return aux;
}

void Tokenizador::TokenizarRapido(const string& str, vector<string>& tokens) const {
    tokens.clear();
    if (str.empty()) {
        return;
    }

    string strNormalizado;
    const string* punteroStr = &str;

    if (pasarAminuscSinAcentos) {
        strNormalizado = Normalizar(str);
        punteroStr = &strNormalizado;
    }

    const char* strData = punteroStr->data();
    size_t len = punteroStr->length();
    size_t i = 0;

    while (i < len) {
        while (i < len && es_delim[(unsigned char)strData[i]]) {
            i++;
        }
        if (i >= len) {
            break;
        }

        size_t start = i;
        while (i < len && !es_delim[(unsigned char)strData[i]]) {
            i++;
        }

        tokens.emplace_back(strData + start, i - start);
    }
}
