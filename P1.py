import struct

class Alumno:
    FORMAT = "is5s11s20s15sif"
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
        data = struct.pack(self.FORMAT, self.codigo, self.nombre.encode('utf-8'), self.apellidos.encode('utf-8'), self.carrera.encode('utf-8'), self.ciclo, self.mensualidad)
        return data

    @staticmethod
    def unpack(data):
        alumno = Alumno(*struct.unpack(Alumno.FORMAT, data))
        return alumno

    def __str__(self):
        return f"Alumno(codigo={self.codigo}, nombre={self.nombre}, apellidos={self.apellidos}, carrera={self.carrera}, ciclo={self.ciclo}, mensualidad={self.mensualidad})"

class FixedRecordFile: #move the last
    def __init__(self, filename:str):
        self.filename = filename
    
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
            #ubicar posicion libre al final
            f.seek(0, 2)
            pos = f.tell() // Alumno.RECORD_SIZE
            #agregar el registro
            f.write(alumno.pack())

    def readRecord(self, pos: int):
        with open(self.filename, 'rb') as f:
            #leer el registro
            f.seek(pos * Alumno.RECORD_SIZE)
            data = f.read(Alumno.RECORD_SIZE)
            if not data:
                return None
            record = Alumno.unpack(data)
            return record

    def remove(self, pos:int):
        with open(self.filename, 'r+b') as f:
            # encontrar registro a eliminar
            # eliminar
            # reeemplazar en la posicion un registro del final
            # cambiar el tamaño del archivo
            

# main
# probar todas las funciones
# ejecutar programa para escribir
# ejecutar programa para leer

def __main__():
    pass
