#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
using namespace std;

struct Record {
    string codigo;
    int ciclo;
    float mensualidad;
    string observaciones;

    string serialize() const {
        string data = codigo + "|" + to_string(ciclo) + "|" + to_string(mensualidad) + "|" + observaciones;
        return data;
    }

    static Record deserialize(const string &data) {
        Record r;
        size_t p1 = data.find("|");
        size_t p2 = data.find("|", p1 + 1);
        size_t p3 = data.find("|", p2 + 1);

        r.codigo = data.substr(0, p1);
        r.ciclo = stoi(data.substr(p1 + 1, p2 - p1 - 1));
        r.mensualidad = stof(data.substr(p2 + 1, p3 - p2 - 1));
        r.observaciones = data.substr(p3 + 1);

        return r;
    }
};

struct HeaderEntry {
    int offset;
    int size;
    bool valid;
    int next_free;
};

struct Header {
    vector<HeaderEntry> entries;
    int head_free = -1;
};

class FileManager {
    string filename;
    Header header;

public:
    FileManager(const string &fname) : filename(fname) {
        ofstream file(filename, ios::binary | ios::trunc); // limpiar archivo
    }

    int add(const Record &r) {
        string data = r.serialize();
        int newSize = data.size();

        // Buscar hueco (first-fit)
        int prev = -1;
        int curr = header.head_free;
        while (curr != -1) {
            HeaderEntry &entry = header.entries[curr];
            if (!entry.valid && entry.size >= newSize) {
                cout << "Reutilizando hueco en entrada " << curr << " offset=" << entry.offset << endl;

                // Ajustar lista libre
                if (prev == -1)
                    header.head_free = entry.next_free;
                else
                    header.entries[prev].next_free = entry.next_free;

                // Si sobra espacio, creamos nuevo hueco
                if (entry.size > newSize) {
                    HeaderEntry newHole;
                    newHole.offset = entry.offset + newSize;
                    newHole.size = entry.size - newSize;
                    newHole.valid = false;
                    newHole.next_free = header.head_free;

                    header.entries.push_back(newHole);
                    header.head_free = header.entries.size() - 1;

                    entry.size = newSize;
                }

                entry.valid = true;
                entry.next_free = -1;

                // Escribir el registro en el archivo
                fstream file(filename, ios::in | ios::out | ios::binary);
                file.seekp(entry.offset, ios::beg);
                file.write(data.c_str(), newSize);
                file.close();

                return curr;
            }
            prev = curr;
            curr = entry.next_free;
        }

        // No hubo hueco -> insertar al final
        ofstream file(filename, ios::binary | ios::app);
        int offset = file.tellp();
        file.write(data.c_str(), newSize);
        file.close();

        HeaderEntry entry{offset, newSize, true, -1};
        header.entries.push_back(entry);
        cout << "Insertado al final en offset " << offset << endl;
        return header.entries.size() - 1;
    }

    void remove(int index) {
        if (index < 0 || index >= (int)header.entries.size()) return;

        HeaderEntry &entry = header.entries[index];
        if (!entry.valid) return;

        entry.valid = false;
        entry.next_free = header.head_free;
        header.head_free = index;

        cout << "Entrada " << index << " eliminada y añadida a lista libre.\n";
    }

    Record read(int index) {
        if (index < 0 || index >= (int)header.entries.size()) throw runtime_error("Índice inválido");

        HeaderEntry &entry = header.entries[index];
        if (!entry.valid) throw runtime_error("Entrada inválida");

        fstream file(filename, ios::in | ios::binary);
        file.seekg(entry.offset, ios::beg);

        vector<char> buffer(entry.size);
        file.read(buffer.data(), entry.size);
        file.close();

        return Record::deserialize(string(buffer.begin(), buffer.end()));
    }

    void printHeader() {
        cout << "\n=== Contenido de la cabecera ===\n";
        cout << "head_free = " << header.head_free << endl;
        for (int i = 0; i < (int)header.entries.size(); i++) {
            auto &e = header.entries[i];
            cout << "Entrada " << i
                 << " -> offset=" << e.offset
                 << " size=" << e.size
                 << " valid=" << e.valid
                 << " next_free=" << e.next_free << endl;
        }
        cout << "================================\n";
    }
};

// ================= MAIN =================

int main() {
    FileManager fm("dataV3.bin");

    Record r1{"A001", 1, 1000, "Alumno uno"};
    Record r2{"A002", 2, 1200, "Alumno dos con texto más largo"};
    Record r3{"A003", 3, 1500, "Tres"};

    cout << "Agregando r1, r2, r3\n";
    int i1 = fm.add(r1);
    int i2 = fm.add(r2);
    int i3 = fm.add(r3);

    fm.printHeader();

    cout << "Eliminando índice 1\n";
    fm.remove(i2);
    fm.printHeader();

    Record r4{"A004", 4, 2100, "Cuarto alumno con observación"};
    cout << "Agregando r4 (debería reutilizar hueco 1 con first-fit)\n";
    int i4 = fm.add(r4);

    fm.printHeader();

    cout << "Leyendo índice " << i4 << ":\n";
    Record rr = fm.read(i4);
    cout << "Codigo: " << rr.codigo << "\n"
         << "Ciclo: " << rr.ciclo << "\n"
         << "Mensualidad: " << rr.mensualidad << "\n"
         << "Observaciones: " << rr.observaciones << "\n"
         << "-------------------------\n";

    return 0;
}
