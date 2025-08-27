import struct
import os

class Alumno:
    FORMAT = "i20s30s20sifi"
    RECORD_SIZE = struct.calcsize(FORMAT)

    def __init__(self, codigo:int, nombre:str, apellidos:str, carrera:str, ciclo:int, mensualidad:float):
        self.codigo = codigo
        self.nombre = nombre
        self.apellidos = apellidos
        self.carrera = carrera
        self.ciclo = ciclo
        self.mensualidad = mensualidad
        self.next_del = 0
        self.pos = None   # siempre existe

    def pack(self):
        return struct.pack(
            self.FORMAT,
            self.codigo,
            self.nombre.encode('utf-8').ljust(5, b'\x00'),
            self.apellidos.encode('utf-8').ljust(11, b'\x00'),
            self.carrera.encode('utf-8').ljust(20, b'\x00'),
            self.ciclo,
            self.mensualidad,
            self.next_del
        )

    @staticmethod
    def unpack(data):
        codigo, nombre, apellidos, carrera, ciclo, mensualidad, next_del = struct.unpack(Alumno.FORMAT, data)
        alumno = Alumno(
            codigo,
            nombre.decode('utf-8').rstrip('\x00'),
            apellidos.decode('utf-8').rstrip('\x00'),
            carrera.decode('utf-8').rstrip('\x00'),
            ciclo,
            mensualidad
        )
        alumno.next_del = next_del
        return alumno

    def __str__(self):
        return f"Alumno(codigo={self.codigo}, nombre={self.nombre}, apellidos={self.apellidos}, carrera={self.carrera}, ciclo={self.ciclo}, mensualidad={self.mensualidad}, next_del={self.next_del})"


class FixedRecordFile:
    def __init__(self, filename:str):
        self.filename = filename
        self.pos_del = -1
    
    def load(self):
        with open(self.filename, 'rb') as f:
            idx = 0
            while True:
                data = f.read(Alumno.RECORD_SIZE)
                if not data:
                    break
                record = Alumno.unpack(data)
                print(f"[{idx}] {record}")
                idx += 1

    def addRecord(self, alumno: Alumno):
        with open(self.filename, 'r+b' if os.path.exists(self.filename) else 'w+b') as f:
            if self.pos_del == -1:
                # Append at end
                f.seek(0, 2)
                pos = f.tell() // Alumno.RECORD_SIZE
                f.write(alumno.pack())
            else:
                # Reuse deleted slot
                f.seek(self.pos_del * Alumno.RECORD_SIZE)
                data = f.read(Alumno.RECORD_SIZE)
                old = Alumno.unpack(data)
                next_pos = old.next_del
                pos = self.pos_del
                f.seek(pos * Alumno.RECORD_SIZE)
                f.write(alumno.pack())
                self.pos_del = next_pos
        alumno.pos = pos
        return pos

    def readRecord(self, pos: int):
        with open(self.filename, 'rb') as f:
            f.seek(pos * Alumno.RECORD_SIZE)
            data = f.read(Alumno.RECORD_SIZE)
            if not data:
                return None
            return Alumno.unpack(data)

    def remove(self, pos:int):
        with open(self.filename, 'r+b') as f:
            f.seek(pos * Alumno.RECORD_SIZE)
            data = f.read(Alumno.RECORD_SIZE)
            if not data:
                return False
            record = Alumno.unpack(data)
            record.next_del = self.pos_del
            self.pos_del = pos
            f.seek(pos * Alumno.RECORD_SIZE)
            f.write(record.pack())
            return True


def main():
    archivo = FixedRecordFile("alumnos.dat")
    
    alumno1 = Alumno(12345, "Juan", "Pérez", "Ing", 5, 1500.0)
    alumno2 = Alumno(67890, "María", "López", "Med", 3, 2000.0)
    alumno3 = Alumno(11111, "Carlos", "Ruiz", "Der", 7, 1800.0)
    
    print("=== Agregando registros ===")
    print(f"Agregado {alumno1.codigo} en posición: {archivo.addRecord(alumno1)}")
    print(f"Agregado {alumno2.codigo} en posición: {archivo.addRecord(alumno2)}")
    print(f"Agregado {alumno3.codigo} en posición: {archivo.addRecord(alumno3)}")

    print("\n=== Mostrando todos los registros ===")
    archivo.load()
    
    print(f"\n=== Eliminando registro en posición 1 (pos_del actual = {archivo.pos_del}) ===")
    archivo.remove(1)
    print(f"Después de eliminar: pos_del actual = {archivo.pos_del}")
    
    print("\n=== Registros después de eliminar ===")
    archivo.load()

    alumno4 = Alumno(22222, "Ana", "Gómez", "Arqu", 2, 1700.0)
    print(f"\n=== Agregando nuevo registro (debería reutilizar posición {archivo.pos_del}) ===")
    print(f"Agregado {alumno4.codigo} en posición: {archivo.addRecord(alumno4)}")

    print("\n=== Registros finales ===")
    archivo.load()

if __name__ == "__main__":
    main()