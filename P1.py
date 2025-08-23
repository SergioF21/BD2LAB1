import struct

class Alumno:
    FORMAT = "i5s11s20sif"  
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
        if isinstance(self.nombre, bytes):
            nombre = self.nombre
            apellidos = self.apellidos
            carrera = self.carrera
        else:
            nombre = self.nombre.encode('utf-8')[:5].ljust(5, b'\x00')  # 5 bytes
            apellidos = self.apellidos.encode('utf-8')[:11].ljust(11, b'\x00')  # 11 bytes
            carrera = self.carrera.encode('utf-8')[:20].ljust(20, b'\x00')  # 20 bytes
        
        data = struct.pack(self.FORMAT, self.codigo, nombre, apellidos, carrera, self.ciclo, self.mensualidad)
        return data

    @staticmethod
    def unpack(data):
        unpacked_data = struct.unpack(Alumno.FORMAT, data)
        alumno = Alumno(*unpacked_data)
        return alumno

    def __str__(self):
        nombre = self.nombre.decode('utf-8').rstrip('\x00') if isinstance(self.nombre, bytes) else self.nombre
        apellidos = self.apellidos.decode('utf-8').rstrip('\x00') if isinstance(self.apellidos, bytes) else self.apellidos
        carrera = self.carrera.decode('utf-8').rstrip('\x00') if isinstance(self.carrera, bytes) else self.carrera
        
        return f"Alumno( codigo={self.codigo}, nombre={nombre}, apellidos={apellidos}, carrera={carrera}, ciclo={self.ciclo}, mensualidad={self.mensualidad})"

class FixedRecordFile:
    def __init__(self, filename:str, real:int = 0):
        self.filename = filename
        self.real = real  

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
        try:
            with open(self.filename, 'r+b') as f:
                pass
        except FileNotFoundError:
            with open(self.filename, 'wb') as f:
                pass
        
        with open(self.filename, 'r+b') as f:
            f.seek(self.real * Alumno.RECORD_SIZE)
            f.write(alumno.pack())
            
            self.real += 1

    def readRecord(self, pos: int):
        with open(self.filename, 'rb') as f:
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
        if pos >= self.real or pos < 0:
            print(f"Posición {pos} fuera de rango (real = {self.real})")
            return False
            
        with open(self.filename, 'r+b') as f:
            if pos < self.real - 1:
                f.seek((self.real - 1) * Alumno.RECORD_SIZE)
                last_record_data = f.read(Alumno.RECORD_SIZE)
                
                f.seek(pos * Alumno.RECORD_SIZE)
                f.write(last_record_data)
            
            self.real -= 1
            


            return True
            

def main():

    archivo = FixedRecordFile("alumnos.dat", 0)  
    
    # Crear algunos alumnos
    alumno1 = Alumno(12345, "Juan", "Pérez", "Ing", 5, 1500.0)
    alumno2 = Alumno(67890, "María", "López", "Med", 3, 2000.0)
    alumno3 = Alumno(11111, "Carlos", "Ruiz", "Der", 7, 1800.0)
    alumno4 = Alumno(22222, "Ana", "García", "Bio", 4, 1700.0)

    # agregamos 
    print("=== Agregando 4 registros ===")
    archivo.addRecord(alumno1)
    archivo.addRecord(alumno2) 
    archivo.addRecord(alumno3)
    archivo.addRecord(alumno4)
    print(f"Total registros agregados: archivo.real = {archivo.real}")
    
    # mostramos todos antes de eliminar
    print("\n=== ANTES DE ELIMINAR - load2() (registros válidos) ===")
    archivo.load2()
    
    # eliminamos
    print("\n=== Eliminando registro en posición 1 (María) ===")
    archivo.remove(1)
    print(f"Después de eliminar: archivo.real = {archivo.real}")
    
    print("\n=== Eliminando registro en posición 2 (Ana, que ahora está en pos 2) ===")
    archivo.remove(2)
    print(f"Después de eliminar: archivo.real = {archivo.real}")
    
    # VEMOS LA DIRENCIA ENTRE BASURA Y LOS CORRECTOS 
    print("\n" + "="*60)
    print("DIFERENCIA ENTRE load() Y load2() DESPUÉS DE ELIMINAR:")
    print("="*60)
    
    print(f"\n=== load() - Lee TODO el archivo (puede mostrar basura) ===")
    archivo.load()
    
    print(f"\n=== load2() - Solo lee {archivo.real} registros válidos ===")
    archivo.load2()
    
    print(f"\n=== RESUMEN ===")
    print(f"archivo.real = {archivo.real} (registros válidos)")
    print("load() mostró registros + posible basura")
    print("load2() mostró solo los registros válidos")

if __name__ == "__main__":
    main()
