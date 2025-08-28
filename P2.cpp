#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

/*
Estructura de registro: Matricula
- codigo (string, tamaño variable)
- ciclo (int)
- mensualidad (float)
- observaciones (string, tamaño variable)
Se almacenan como:
[int tamañoCodigo][codigo][int ciclo][float mensualidad][int tamañoObs][observaciones]
*/

class Matricula {
public:
    string codigo;
    int ciclo;
    float mensualidad;
    string observaciones;

    Matricula() : ciclo(0), mensualidad(0.0) {}
    Matricula(string c, int ci, float m, string o)
        : codigo(c), ciclo(ci), mensualidad(m), observaciones(o) {}

    void display() {
        cout << "Codigo: " << codigo << endl;
        cout << "Ciclo: " << ciclo << endl;
        cout << "Mensualidad: " << mensualidad << endl;
        cout << "Observaciones: " << observaciones << endl;
        cout << "-----------------------------" << endl;
    }

    // Serialización
    void pack(ofstream &out) {
        int len = codigo.size();
        out.write((char*)&len, sizeof(int));
        out.write(codigo.c_str(), len);

        out.write((char*)&ciclo, sizeof(int));
        out.write((char*)&mensualidad, sizeof(float));

        len = observaciones.size();
        out.write((char*)&len, sizeof(int));
        out.write(observaciones.c_str(), len);
    }

    // Deserialización
    static Matricula unpack(ifstream &in) {
        int len;
        string codigo, observaciones;
        int ciclo;
        float mensualidad;

        // codigo
        in.read((char*)&len, sizeof(int));
        if (!in) return Matricula();
        codigo.resize(len);
        in.read(&codigo[0], len);

        // ciclo
        in.read((char*)&ciclo, sizeof(int));

        // mensualidad
        in.read((char*)&mensualidad, sizeof(float));

        // observaciones
        in.read((char*)&len, sizeof(int));
        observaciones.resize(len);
        in.read(&observaciones[0], len);

        return Matricula(codigo, ciclo, mensualidad, observaciones);
    }
};

/*
Clase RecordFile
Maneja:
- archivo de datos binario (matriculas.dat)
- archivo de cabecera con offsets (cabecera.dat)
*/

class RecordFile {
private:
    string filename;    // datos
    string headername;  // metadata (índices)

public:
    RecordFile(string fname) : filename(fname), headername("cabecera.dat") {}

    // Agregar registro O(1)
    void add(Matricula record) {
        ofstream out(filename, ios::binary | ios::app);
        if (!out) { cerr << "Error abriendo archivo de datos\n"; return; }

        auto pos = out.tellp();   // posición donde empieza el registro
        record.pack(out);
        out.close();

        // guardamos el offset en cabecera
        ofstream fc(headername, ios::binary | ios::app);
        int offset = (int)pos;
        fc.write((char*)&offset, sizeof(int));
        fc.close();

        cout << "Record agregado en posición física " << offset << endl;
    }

    // Leer un registro O(1)
    Matricula readRecord(int pos) {
        ifstream fc(headername, ios::binary);
        if (!fc) { cerr << "No se pudo abrir cabecera\n"; return Matricula(); }

        fc.seekg(pos * sizeof(int), ios::beg);
        int offset;
        fc.read((char*)&offset, sizeof(int));
        fc.close();

        if (offset == -1) {
            cerr << "Registro eliminado en posición lógica " << pos << endl;
            return Matricula();
        }

        ifstream in(filename, ios::binary);
        in.seekg(offset, ios::beg);
        Matricula r = Matricula::unpack(in);
        in.close();

        return r;
    }

    // Cargar todos los registros
    void load() {
        ifstream fc(headername, ios::binary);
        if (!fc) { cerr << "No se pudo abrir cabecera\n"; return; }

        vector<int> offsets;
        int offset;
        while (fc.read((char*)&offset, sizeof(int))) {
            offsets.push_back(offset);
        }
        fc.close();

        cout << "=== Todos los registros ===" << endl;
        ifstream in(filename, ios::binary);
        for (int pos = 0; pos < (int)offsets.size(); pos++) {
            if (offsets[pos] == -1) continue; // eliminado
            in.seekg(offsets[pos], ios::beg);
            Matricula r = Matricula::unpack(in);
            r.display();
        }
        in.close();
    }

    // Eliminar registro O(1): marcar en metadata
    void remove(int pos) {
        fstream fc(headername, ios::binary | ios::in | ios::out);
        if (!fc) { cerr << "No se pudo abrir cabecera\n"; return; }

        fc.seekp(pos * sizeof(int), ios::beg);
        int invalid = -1;
        fc.write((char*)&invalid, sizeof(int));
        fc.close();

        cout << "Registro eliminado en posición lógica " << pos << endl;
    }
};

void showHeader() {
    ifstream cab("cabecera.dat", ios::binary);
    if (!cab.is_open()) {
        cout << "[!] No se pudo abrir cabecera.dat" << endl;
        return;
    }
    cout << "\n=== Contenido de la cabecera ===" << endl;
    int pos;
    int i = 0;
    while (cab.read((char*)&pos, sizeof(int))) {
        cout << "Registro lógico " << i++ << " -> Posición física: " << pos << endl;
    }
    cout << "================================" << endl;
    cab.close();
}


/*
Pruebas funcionales (main)
*/

int main() {
    RecordFile recordFile("matriculas.dat");

    // Agregar registros
    Matricula r1("A001", 1, 1500.0, "Primer alumno");
    Matricula r2("A002", 2, 1800.5, "Segundo alumno");
    Matricula r3("A003", 3, 1600.0, "Tercer alumno");

    cout << "Mostrando cabecera inicial (vacía):" << endl;
    showHeader();

    recordFile.add(r1);
    recordFile.add(r2);
    recordFile.add(r3);

    cout << "Cabecera después de agregar 3 registros:" << endl;
    showHeader();
    
    recordFile.load();   // mostrar todos

    cout << endl << "Leyendo registro 1:" << endl;
    Matricula r = recordFile.readRecord(1);
    r.display();

    cout << endl << "Eliminando registro 1" << endl;
    recordFile.remove(1);
    

    cout << endl << "Cargando después de eliminar:" << endl;
    recordFile.load();

    cout << "Cabecera después de eliminar registro 1:" << endl;
    showHeader();
    
    return 0;
}
