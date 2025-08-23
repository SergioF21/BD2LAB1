import struct

class Alumno:
    FORMAT = "s5s11s20s15sifi"
    RECORD_SIZE = struct.calcsize(FORMAT)
    def __init__(self, codigo:int, nombre:str, apellidos: str, carrera: str, ciclo: int, mensualidad:float):
        self.codigo = codigo
        self.nombre = nombre
        self.apellidos = apellidos
        self.carrera = carrera
        self.ciclo = ciclo
        self.mensualidad = mensualidad
        self.next_del = 0

    def pack(self):
        data = struct.pack(self.FORMAT, self.codigo, self.nombre.encode('utf-8'), self.apellidos.encode('utf-8'), self.carrera.encode('utf-8'), self.ciclo, self.mensualidad, self.next_del)
        return data

    @staticmethod
    def unpack(data):
        alumno = Alumno(*struct.unpack(Alumno.FORMAT, data))
        return alumno

    def __str__(self):
        return f"Alumno(codigo={self.codigo}, nombre={self.nombre}, apellidos={self.apellidos}, carrera={self.carrera}, ciclo={self.ciclo}, mensualidad={self.mensualidad})"

class FixedRecordFile:
    def __init__(self, filename:str):
        self.filename = filename
        self.pos_del = -1
    
    def load(self):
        with open(self.filename, 'rb') as f:
            while True:
                data = f.read(Alumno.RECORD_SIZE)
                if not data:
                    break
                record = Alumno.unpack(data)
                print(record)
        

    def addRecord(self, alumno: Alumno):
        with open(self.filename, 'ab') as f:
            f.seek(0, 2)
            while self.pos_del != -1:
                self.pos_del = f.seek(self.pos_del * Alumno.RECORD_SIZE)
                self.pos_del = f.read(Alumno.RECORD_SIZE - struct.calcsize('i'))
                self.pos_del = struct.unpack('i', self.pos_del)[0]
            f.write(alumno.pack())

    def readRecord(self, pos: int):
        with open(self.filename, 'rb') as f:
            f.seek(pos * Alumno.RECORD_SIZE)
            data = f.read(Alumno.RECORD_SIZE)
            if not data:
                return None
            record = Alumno.unpack(data)
            return record

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


# main
# probar todas las funciones
# ejecutar programa para escribir
# ejecutar programa para leer

def main():
    # Crear una instancia del archivo de registros fijos
    archivo = FixedRecordFile("alumnos.dat")  # Empezamos con 0 registros
    
    # Crear algunos alumnos
    alumno1 = Alumno(12345, "Juan", "Pérez", "Ing", 5, 1500.0)
    alumno2 = Alumno(67890, "María", "López", "Med", 3, 2000.0)
    alumno3 = Alumno(11111, "Carlos", "Ruiz", "Der", 7, 1800.0)
    
    # Agregar registros
    print("=== Agregando registros ===")
    archivo.addRecord(alumno1)
    print(f"Alumno1 guardado en posición real: {alumno1.real}, archivo.real = {archivo.real}")
    
    archivo.addRecord(alumno2) 
    print(f"Alumno2 guardado en posición real: {alumno2.real}, archivo.real = {archivo.real}")
    
    archivo.addRecord(alumno3)
    print(f"Alumno3 guardado en posición real: {alumno3.real}, archivo.real = {archivo.real}")
    
    # Mostrar todos los registros
    print("\n=== Mostrando todos los registros ===")
    archivo.load()
    
    # Eliminar un registro (posición 1)
    print(f"\n=== Eliminando registro en posición 1 (archivo.real = {archivo.real}) ===")
    archivo.remove(1)
    print(f"Después de eliminar: archivo.real = {archivo.real}")
    
    # Mostrar registros después de la eliminación
    print("\n=== Registros después de eliminar ===")
    archivo.load()

if __name__ == "__main__":
    main()