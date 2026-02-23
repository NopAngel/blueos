import struct
import os

def create_initrd(source_dir, output_file):
    files = [f for f in os.listdir(source_dir) if os.path.isfile(os.path.join(source_dir, f))]
    n_files = min(len(files), 32)
    
    # 4 bytes (nfiles) + 32 archivos * 72 bytes cada uno
    header_size = 4 + (32 * 72)
    blob = bytearray(header_size)
    
    # Escribir nfiles (Little Endian)
    struct.pack_into("<I", blob, 0, n_files)
    
    current_offset = header_size
    data_blobs = bytearray()

    for i in range(32):
        base_pos = 4 + (i * 72)
        if i < n_files:
            f_name = files[i]
            f_path = os.path.join(source_dir, f_name)
            with open(f_path, "rb") as f:
                f_data = f.read()
            
            f_size = len(f_data)
            
            # Nombre (64 bytes rellenos de ceros)
            name_bytes = f_name.encode('ascii')[:63]
            blob[base_pos : base_pos + len(name_bytes)] = name_bytes
            
            # Size (4 bytes) en la posición base + 64
            struct.pack_into("<I", blob, base_pos + 64, f_size)
            # Offset (4 bytes) en la posición base + 68
            struct.pack_into("<I", blob, base_pos + 68, current_offset)
            
            data_blobs += f_data
            current_offset += f_size
        else:
            # Rellenar con ceros el resto de las entradas
            pass

    with open(output_file, "wb") as f:
        f.write(blob)
        f.write(data_blobs)
    print(f"Initrd listo: {output_file} ({n_files} archivos)")

if __name__ == "__main__":
    if not os.path.exists("./initrd_root"):
        os.makedirs("./initrd_root")
        with open("./initrd_root/hello.txt", "w") as f:
            f.write("¡Hola desde el Initrd de BlueOS!")
    create_initrd("./initrd_root", "my_initrd.img")