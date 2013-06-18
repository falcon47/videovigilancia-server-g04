#ifndef RIFF_H
#define RIFF_H

#include <stdint.h>

#define SIZEOF_RIFF 12
#define SIZEOF_HEADER 4084
#define SIZEOF_CLIST 16777228
#define SIZEOF_CAM 256
#define SIZEOF_IMG 24

struct RIFF             // Cabecera riff
{
    uint32_t type;
    uint32_t size;
    uint32_t listType;
    uint8_t  data[];
};  // tamaño del archivo

struct HEADER           // Cabecera con metadatos
{
    uint32_t type;
    uint32_t size;
    uint64_t num_img;      // id de imagen por la que va
    uint32_t num_clist;    // id de camara por la que va (CXXX)
    uint8_t padding[4064]; // Relleno para alinear a 4k pagina
};  // 4 kbytes - 12 bytes de riff

struct C_list           // lista de imagenes con cabecera de metadatos de camara
{
    uint32_t type;
    uint32_t size;
    uint32_t listType;
    uint8_t  data[16777216 - 12];
};  // 16 Mbytes

struct CAM              // Metadatos de camara
{
    uint32_t type;
    uint32_t size;
    uint8_t name[236];     // Nombre de la camara
    uint64_t size_taken;   // Espacio ocupado por las imagenes
    uint32_t ultimo;       // Si es el ultimo list de este id de cam
};  // 256 bytes

struct IMG              // Metadatos de imagenes
{
    uint32_t type;
    uint32_t size;
    uint64_t timestamp;   // timestamp
    uint64_t num_img;     // id de imagen
    uint32_t cuadros_size; // tamaño de los cuadros
    uint8_t cuadros[];     // cuadros serializados
} __attribute__((packed)) ;  // 16 + x

uint32_t char2int(const char * b);

char * int2char4(uint32_t b);

void* alignPointer(const void* pointer, int alignment);

int impar(const void * ptr);

#endif // RIFF_H
