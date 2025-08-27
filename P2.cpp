#include <iostream>
#include <cstring>
using namespace std;

class Matricula {
    private:
        char* codigo;
        int ciclo;
        float mensualidad;
        char*  observaciones;
    public:
        Matricula(char* codigo, int ciclo, float mensualidad, char*  observaciones) {
            this->codigo = codigo;
            this->ciclo = ciclo;
            this->mensualidad = mensualidad;
            this->observaciones = observaciones;
        };
        ~Matricula();
};

/*
Se le pide implementar un programa para leer y escribir registros de longitud variable en un archivo binario 
usando el tamaño del dato como seperador:

1. manejar un archivo adicional para indicar la posicion incial de cada registro
    Evaluar si es necesario guardar también el tamaño del registro
2. Implementar las siguiente funciones:
    a. load():
    b. add(record):
    c. readRecord(pos):
    d. remove(pos):
3. REalizar las pruebas funcionales de cada metodo.
*/

class RecordFile {
    private:
        char* filename;
    public:
        RecordFile(char* filename) {
            this->filename = filename;
        };
        ~RecordFile();
        void load();
        void add(Matricula record);
        Matricula readRecord(int pos);
        void remove(int pos);
};

int main() {

    return 0;
}