#include <iostream>
#include <fstream>
#include <istream>
#include <ostream>
#include <string>
#include <cstring>
using namespace std;

class Matricula {
    public:
        char* codigo;
        int ciclo;
        float mensualidad;
        char*  observaciones;
        Matricula(char* codigo, int ciclo, float mensualidad, char*  observaciones) {
            this->codigo = codigo;
            this->ciclo = ciclo;
            this->mensualidad = mensualidad;
            this->observaciones = observaciones;
        };
        ~Matricula();
        void display() {
            cout << "Codigo: " << codigo << endl;
            cout << "Ciclo: " << ciclo << endl;
            cout << "Mensualidad: " << mensualidad << endl;
            cout << "Observaciones: " << observaciones << endl;
        };
        int getsizeof() {
            return sizeof(codigo) + sizeof(ciclo) + sizeof(mensualidad) + sizeof(observaciones);
        }
        char* pack(){
            int size = getsizeof();
            char* buffer = new char[size];
            memcpy(buffer, codigo, sizeof(codigo));
            memcpy(buffer + sizeof(codigo), &ciclo, sizeof(ciclo));
            memcpy(buffer + sizeof(codigo) + sizeof(ciclo), &mensualidad, sizeof(mensualidad));
            memcpy(buffer + sizeof(codigo) + sizeof(ciclo) + sizeof(mensualidad), observaciones, sizeof(observaciones));
            return buffer;
        }
};

/*
Se le pide implementar un programa para leer y escribir registros de longitud variable en un archivo binario 
usando el tamaño del dato como separador:

1. manejar un archivo adicional para indicar la posicion incial de cada registro
    Evaluar si es necesario guardar también el tamaño del registro
2. Implementar las siguiente funciones:
    a. load(): devolver todos los registros del archivo
    b. add(record): agregar un nuevo registro al archivo O(1)
    c. readRecord(pos): Obtiene el registro de la posicion "pos" O(1).
    d. remove(pos): elimina el registro de la posicion "pos" proponer una estrategia eficiente
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

Matricula::~Matricula() {
    delete[] codigo;
    delete[] observaciones;
}

RecordFile::~RecordFile() {
    delete[] filename;
}

void RecordFile::load() {
    // Implementation for loading records from the file
    ifstream ptrFile(filename, ios::binary);
    if (ptrFile.is_open()) {
        // Logic to read all records from the file
        for (int i = 0; i < 10; i++) { // Placeholder loop
            // Read and display each record
            Matricula record(nullptr, 0, 0.0, nullptr);
            record = readRecord(i * sizeof(Matricula)); // Placeholder
        }
        cout << "File loaded successfully." << endl; // Placeholder
        ptrFile.close();
    } else {
        cerr << "Error opening file." << endl;
    }
}

void RecordFile::add(Matricula record) {
    // Implementation for adding a new record to the file
    ofstream ptrFile(filename, ios::binary);
    if (ptrFile){
        // Logic to write the record to the file
        ptrFile.seekp(0, ios::end); // Move to the end for appending
        auto pos = ptrFile.tellp();
        ptrFile.write(record.pack(), record.getsizeof()); 
        ofstream fc ("cabecera.dat", ios::binary | ios::app);
        fc.seekp(0, ios::end);
        fc.write((char*)&pos, sizeof(int));
        fc.write((char*)&record.getsizeof(), sizeof(int));
        cout << "Record added at position " << pos << endl; // Placeholder
        ptrFile.close(); fc.close();
    } else {
        cerr << "Error opening file." << endl;
    }
}

Matricula RecordFile::readRecord(int pos) {
    // Implementation for reading a record from a specific position
    Matricula record(nullptr, 0, 0.0, nullptr);
    ifstream ptrFile(filename, ios::binary);
    if (ptrFile.is_open()) {
        // Logic to read the record at position 'pos'
        ptrFile.seekg(pos*record.getsizeof(), ios::beg);
        ptrFile.read((char*)(&record), record.getsizeof());
        // Read the record data here
        cout<<"Record read from position " << pos << endl;
        record.display(); // Placeholder
        ptrFile.close();
        return record;
    } else {
        cerr << "Error opening file." << endl;
    }
    return Matricula(nullptr, 0, 0.0, nullptr); // Placeholder return
}

void RecordFile::remove(int pos) {
    // Implementation for removing a record from a specific position
    ofstream ptrFile(filename, ios::binary | ios::in | ios::out);
    if (!ptrFile.is_open()) {
        cerr << "Error opening file." << endl;
        return;
    }
    // Logic to remove the record at position 'pos'
    ptrFile.close();
    cout << "Record removed from position " << pos << endl; // Placeholder
}

int main() {
    string filename = "matriculas.dat";
    RecordFile recordFile((char*)filename.c_str());
    recordFile.load();
    Matricula record1((char*)"A001", 1, 1500.0, (char*)"First record");
    recordFile.add(record1);
    Matricula readRecord = recordFile.readRecord(0);


    return 0;
}