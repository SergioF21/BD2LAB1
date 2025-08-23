import struct

class Alumno:
    FORMAT = "is5s11s20s15sif"  # Sin el campo real en el formato
    RECORD_SIZE = struct.calcsize(FORMAT)
    def __init__(self, codigo:int, nombre:str, apellidos: str, carrera: str, ciclo: int, mensualidad:float):
        self.real = 0  # Se asignará cuando se agregue al archivo
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
        unpacked_data = struct.unpack(Alumno.FORMAT, data)
        alumno = Alumno(*unpacked_data)
        return alumno

    def __str__(self):
        return f"Alumno(codigo={self.codigo}, nombre={self.nombre}, apellidos={self.apellidos}, carrera={self.carrera}, ciclo={self.ciclo}, mensualidad={self.mensualidad})"

class FixedRecordFile: #move the last
    def __init__(self, filename:str, real:int = 0):
        self.filename = filename
        self.real = real  # Número de registros válidos en el archivo

    def load(self):
        with open(self.filename, 'rb') as f:
            while True:
                data = f.read(Alumno.RECORD_SIZE)
                if not data:
                    break
                record = Alumno.unpack(data)
                print(record)

    def load2(self):
        with open(self.filename, 'rb') as f:
            pos = 0
            while pos < self.real:
                data = f.read(Alumno.RECORD_SIZE)
                if not data:
                    break
                record = Alumno.unpack(data)
                print(record)
                pos += 1
                


    def addRecord(self, alumno: Alumno):
        with open(self.filename, 'r+b') as f:
            # Asignar la posición real al alumno (siguiente posición disponible)
            alumno.real = self.real
            
            # Ir directamente a la posición indicada por alumno.real
            f.seek(alumno.real * Alumno.RECORD_SIZE)
            f.write(alumno.pack())
            
            # Incrementar el contador de registros reales
            self.real += 1

    def readRecord(self, pos: int):
        with open(self.filename, 'rb') as f:
            #leer el registro en la posición especificada
            f.seek(pos * Alumno.RECORD_SIZE)
            data = f.read(Alumno.RECORD_SIZE)
            if not data:
                return None
            record = Alumno.unpack(data)
            return record
    
    def readRecordByReal(self, real_pos: int):
        """Lee un registro usando su valor real como posición"""
        return self.readRecord(real_pos)

    def remove(self, pos):
        """Elimina un registro: mueve el último registro a la posición a eliminar y reduce self.real en 1"""
        if pos > self.real or pos < 0:
            print(f"Posición {pos} fuera de rango (real = {self.real})")
            return False
            
        with open(self.filename, 'r+b') as f:
            # Si no es el último registro, mover el último registro a la posición a eliminar
            if pos <= self.real - 1:
                # Leer el último registro (posición self.real - 1)
                f.seek((self.real - 1) * Alumno.RECORD_SIZE)
                last_record_data = f.read(Alumno.RECORD_SIZE)
                
                # Desempaquetar el último registro y actualizar su valor real
                last_record = Alumno.unpack(last_record_data)
                last_record.real = pos  # Actualizar su posición real
                
                # Escribir el último registro en la posición a eliminar
                f.seek(pos * Alumno.RECORD_SIZE)
                f.write(last_record.pack())
            
            # Reducir el contador de registros reales en 1
            self.real -= 1
            
            # Truncar el archivo para que coincida con self.real
            new_size = self.real * Alumno.RECORD_SIZE
            f.truncate(new_size)
            return True
            

def main():
    # Crear una instancia del archivo de registros fijos
    archivo = FixedRecordFile("alumnos.dat", 0)  # Empezamos con 0 registros
    
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
