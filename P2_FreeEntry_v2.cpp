#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

struct Record {
    string codigo;
    int ciclo;
    double mensualidad;
    string observaciones;

    // serializa a string fijo
    string serialize() const {
        string data = codigo + "|" + to_string(ciclo) + "|" + to_string(mensualidad) + "|" + observaciones;
        return data;
    }

    static Record deserialize(const string& data) {
        Record r;
        size_t p1 = data.find('|');
        size_t p2 = data.find('|', p1 + 1);
        size_t p3 = data.find('|', p2 + 1);

        r.codigo = data.substr(0, p1);
        r.ciclo = stoi(data.substr(p1 + 1, p2 - p1 - 1));
        r.mensualidad = stod(data.substr(p2 + 1, p3 - p2 - 1));
        r.observaciones = data.substr(p3 + 1);
        return r;
    }
};

struct HeaderEntry {
    long offset;   // posición en archivo
    int size;      // tamaño del registro
    bool valid;    // 1 válido, 0 eliminado
    int next_free; // siguiente libre (si eliminado)
};

struct Header {
    vector<HeaderEntry> entries;
    int head_free; // índice del primer hueco

    Header() { head_free = -1; }

    void show() {
        cout << "\n=== Contenido de la cabecera ===\n";
        cout << "head_free = " << head_free << "\n";
        for (int i = 0; i < (int)entries.size(); i++) {
            cout << "Entrada " << i << " -> offset=" << entries[i].offset
                 << " size=" << entries[i].size
                 << " valid=" << entries[i].valid
                 << " next_free=" << entries[i].next_free << "\n";
        }
        cout << "================================\n";
    }
};

class FileManager {
    string filename;
    fstream file;
    Header header;

public:
    FileManager(const string& fname) : filename(fname) {
        file.open(filename, ios::in | ios::out | ios::binary | ios::trunc);
        if (!file.is_open()) {
            cerr << "Error abriendo archivo\n";
            exit(1);
        }
    }

    void add(const Record& r) {
        string data = r.serialize();
        int sz = data.size() + 1;

        int pos = -1;
        if (header.head_free != -1) {
            // Reutilizar hueco libre
            pos = header.head_free;
            header.head_free = header.entries[pos].next_free;

            cout << "Reutilizando hueco en entrada " << pos
                 << " offset=" << header.entries[pos].offset << "\n";

            file.seekp(header.entries[pos].offset);
            file.write(data.c_str(), sz);

            header.entries[pos].size = sz;
            header.entries[pos].valid = true;
            header.entries[pos].next_free = -1;
        } else {
            // Insertar al final
            file.seekp(0, ios::end);
            long offset = file.tellp();
            file.write(data.c_str(), sz);

            HeaderEntry he{offset, sz, true, -1};
            header.entries.push_back(he);

            cout << "Insertado al final en offset " << offset << "\n";
        }
    }

    void remove(int idx) {
        if (idx < 0 || idx >= (int)header.entries.size() || !header.entries[idx].valid) {
            cout << "No se puede eliminar índice " << idx << "\n";
            return;
        }

        header.entries[idx].valid = false;
        header.entries[idx].next_free = header.head_free;
        header.head_free = idx;

        cout << "Entrada " << idx << " eliminada y añadida a lista libre.\n";
    }

    void read(int idx) {
        if (idx < 0 || idx >= (int)header.entries.size()) {
            cout << "Índice inválido\n";
            return;
        }
        if (!header.entries[idx].valid) {
            cout << "Entrada " << idx << " está eliminada.\n";
            return;
        }

        file.seekg(header.entries[idx].offset);
        vector<char> buf(header.entries[idx].size);
        file.read(buf.data(), header.entries[idx].size);

        string data(buf.data());
        Record r = Record::deserialize(data);

        cout << "Leyendo entrada " << idx << ":\n";
        cout << "Codigo: " << r.codigo << "\n";
        cout << "Ciclo: " << r.ciclo << "\n";
        cout << "Mensualidad: " << r.mensualidad << "\n";
        cout << "Observaciones: " << r.observaciones << "\n";
        cout << "-------------------------\n";
    }

    void showHeader() { header.show(); }
};

int main() {
    FileManager fm("data2.dat");

    Record r1{"A001", 1, 1500.0, "Primer alumno"};
    Record r2{"A002", 2, 1800.5, "Segundo alumno"};
    Record r3{"A003", 3, 2000.0, "Tercer alumno"};
    Record r4{"A004", 4, 2100.0, "Cuarto alumno"};

    cout << "Agregando r1, r2, r3\n";
    fm.add(r1);
    fm.add(r2);
    fm.add(r3);
    fm.showHeader();

    cout << "\nEliminando índice 1\n";
    fm.remove(1);
    fm.showHeader();

    cout << "\nAgregando r4 (debería reutilizar hueco 1)\n";
    fm.add(r4);
    fm.showHeader();

    cout << "\nLeyendo índice 1:\n";
    fm.read(1);

    return 0;
}
