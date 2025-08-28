#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

string datafile = "matriculas.dat";
string headerfile = "cabecera.dat";

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
    void pack(fstream &out) {
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
    static Matricula unpack(fstream &in) {
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
    int getsizeof() {
        return sizeof(int) + codigo.size() + sizeof(int) + sizeof(float) + sizeof(int) + observaciones.size();
    }
};

struct HeaderEntry {
    int offset; // posición en el archivo de datos
    int size; // tamaño del registro
    bool valid; // si el registro está activo o eliminado
};

struct FreeEntry {
    int offset;
    int size;
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
    string freelistname;

public:
    RecordFile(string fname) : filename(fname), headername(headerfile), freelistname("libres.dat") {}

    vector<FreeEntry> loadFreeList() {
        ifstream f(freelistname, ios::binary);
        vector<FreeEntry> freeList;
        if (!f.is_open()) return freeList;
        FreeEntry e;
        while (f.read((char*)&e, sizeof(FreeEntry))) {
            freeList.push_back(e);
        }
        return freeList; 
    }

    void saveFreeList(const vector<FreeEntry>& freeList) {
        ofstream f(freelistname, ios::binary | ios::trunc);
        for (const auto& e : freeList) {
            f.write((char*)&e, sizeof(FreeEntry));
        }
    }

    // Agregar registro O(1)
    void add(Matricula record) {
        int newSize = record.getsizeof();
        vector<FreeEntry> freeList = loadFreeList();

        int idx = -1;
        for (size_t i = 0; i < freeList.size(); i++) {
            if (freeList[i].size >= newSize) {
                idx = i;
                break;  
            }
        }

        int offset;
        if (idx != -1) {
            // usar hueco
            offset = freeList[idx].offset;
            fstream out(filename, ios::in | ios::out | ios::binary);
            out.seekp(offset, ios::beg);
            record.pack(out);
            out.close();

            // actualizar lista libre
            int leftover = freeList[idx].size - newSize;
            if (leftover > 0) {
                freeList[idx].offset += newSize;
                freeList[idx].size = leftover;
            } else {
                freeList.erase(freeList.begin() + idx);
            }
            saveFreeList(freeList);
            
            // actualizar cabecera
            HeaderEntry e{offset, newSize, true};
            fstream fc(headername, ios::in | ios::out | ios::binary);
            fc.write((char*)&e,sizeof(HeaderEntry));
            fc.close();
            cout<<"Registro agregado reutilizando hueco en offset "<<offset<<endl;
        } else {
            // append
            fstream out(filename, ios::app | ios::binary);
            offset = (int)out.tellp();
            record.pack(out);
            int size = (int)out.tellp() - offset;
            out.close();

            HeaderEntry newEntry{offset, size, true};
            fstream fc2(headername, ios::app | ios::binary);
            fc2.write((char*)&newEntry, sizeof(HeaderEntry));
            fc2.close();

            cout << "Registro agregado al final en offset " << offset << endl;
        }
    }


    // Leer un registro O(1)
    Matricula readRecord(int pos) {
        fstream fc(headerfile, ios::in | ios::binary);
        fc.seekg(pos * sizeof(HeaderEntry), ios::beg);
        HeaderEntry entry;
        fc.read((char*)&entry, sizeof(HeaderEntry));
        fc.close();

        if (!entry.valid) {
            cerr << "Registro eliminado en posición " << pos << endl;
            return Matricula();
        }

        fstream in(datafile, ios::in | ios::binary);
        in.seekg(entry.offset, ios::beg);

        // Proteger contra basura si sobran bytes
        streampos start = in.tellg();
        Matricula r = Matricula::unpack(in);
        streampos end = in.tellg();

        if ((end-start)>entry.size){
            cerr << "[!] Advertencia: se leyó más allá del tamaño registrado" << endl;
        }
        

        in.close();
        return r;
    }

    // Cargar todos los registros
    void load() {
        ifstream fc(headername, ios::binary);
        if (!fc) { cerr << "No se pudo abrir cabecera\n"; return; }

        cout << "=== Todos los registros ===" << endl;
        fstream in(filename, ios::binary);
        HeaderEntry e;
        int pos = 0;
        while (fc.read((char*)&e, sizeof(HeaderEntry))) {
            if (!e.valid) { 
                pos++;
                continue; 
            }
            in.seekg(e.offset, ios::beg);
            Matricula r = Matricula::unpack(in);
            cout << "Registro lógico " << pos << ":" << endl;
            r.display();
            pos++;
        }
        in.close();
        fc.close();
    }


    // Eliminar registro O(1): marcar en metadata
    void remove(int pos) {
        fstream fc(headerfile, ios::in | ios::out | ios::binary);
        fc.seekg(pos * sizeof(HeaderEntry), ios::beg);
        HeaderEntry entry;
        fc.read((char*)&entry, sizeof(HeaderEntry));
        if (!fc) return;

        if (!entry.valid) {
            cerr << "Registro ya eliminado en posición " << pos << endl;
            return;
        }

        entry.valid = false;
        fc.seekp(pos * sizeof(HeaderEntry), ios::beg);
        fc.write((char*)&entry, sizeof(HeaderEntry));
        fc.close();
        
        // añadir hueco a la lista libre
        vector<FreeEntry> freeList = loadFreeList();
        freeList.push_back(FreeEntry{entry.offset, entry.size});
        saveFreeList(freeList);

        cout << "Registro en posición lógica " << pos << " marcado como eliminado." << endl;
    }
};

void showHeader() {
    ifstream cab(headerfile, ios::binary);
    if (!cab.is_open()) {
        cout << "[!] No se pudo abrir cabecera.dat" << endl;
        return;
    }
    cout << "\n=== Contenido de la cabecera ===" << endl;
    HeaderEntry e;
    int i = 0;
    while (cab.read((char*)&e, sizeof(HeaderEntry))) {
        cout << "Registro lógico " << i++
             << " -> offset=" << e.offset
             << " size=" << e.size
             << " valid=" << e.valid << endl;
    }
    cout << "================================" << endl;
    cab.close();
}


void checkHeaderIntegrity(string datafile, string headerfile) {
    ifstream inData(datafile, ios::binary);
    ifstream inHeader(headerfile, ios::binary);
    if (!inData || !inHeader) {
        cout << "[!] No se pudo abrir archivos para verificación" << endl;
        return;
    }

    cout << "\n=== Verificación de integridad de cabecera ===" << endl;
    HeaderEntry e;
    int idx = 0;
    inData.seekg(0, ios::end);
    int filesize = inData.tellg();

    while (inHeader.read((char*)&e, sizeof(HeaderEntry))) {
        if (!e.valid) {
            cout << "Entrada " << idx << " marcada como eliminada ✔ (offset=" << e.offset << ", size=" << e.size << ")" << endl;
        } else if (e.offset >= 0 && e.offset + e.size <= filesize) {
            cout << "Entrada " << idx << " válida (offset=" << e.offset << ", size=" << e.size << ")" << endl;
        } else {
            cout << "Entrada " << idx << " apunta fuera de rango ✘" << endl;
        }
        idx++;
    }
    cout << "==============================================" << endl;
}




/*
Pruebas funcionales (main)
*/

int main() {
    ofstream cab(headerfile, ios::binary | ios::trunc);
    ofstream data(datafile, ios::binary | ios::trunc);
    cab.close();
    data.close();

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

    recordFile.add(Matricula("A004", 4, 1700.0, "Nuevo alumno"));
    recordFile.load();

    cout << "Cabecera después de agregar otro registro:" << endl;
    showHeader();

    cout << "Leyendo registro 1 (eliminado):" << endl;
    Matricula r_elim = recordFile.readRecord(1);
    if (r_elim.codigo.empty()) {
        cout << "Registro 1 está eliminado." << endl;
    } else {
        r_elim.display();
    }
    
    checkHeaderIntegrity("matriculas.dat", "cabecera.dat");

    return 0;
}
