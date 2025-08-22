import struct

class Alumno:
    FORMAT = "i11s20s15si fi"
    RECORD_SIZE = struct.calcsize(FORMAT)
    def __init__(self, codigo:int[5], nombre:str[11], apellidos: str[20], carrera: str[15], ciclo: int, mensualidad:float):
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

class FixedRecordFile:
    def __init__(self, filename:str):
        self.filename = filename
    
    def load(self):
        pass

    def addRecord(self, alumno: Alumno):
        pass

    def readRecord(self, pos: int)
        pass

    def remove(self, pos:int):
        pass

# main
# probar todas las funciones
# ejecutar programa para escribir
# ejecutar programa para leer

def __main__():
    pass
