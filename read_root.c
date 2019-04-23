#include <stdio.h>
#include <stdlib.h>

typedef struct {
	unsigned char first_byte;
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	unsigned short starting_cluster;
	unsigned int file_size;
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3]; 
    unsigned char oem[8]; 
    unsigned short sector_size; 
    unsigned char sectores_por_cluster; 
    unsigned short sectores_reservados; 
    unsigned char cantidad_tablas_fats; 
    unsigned short root_entries; 
    unsigned short cantidad_sectores; 
    unsigned char media_descriptor[1]; 
    unsigned short tamanio_fat; 
    unsigned short sectores_por_track; 
    unsigned short cantidad_heads; 
    unsigned int sectores_ocultos:32; 
    unsigned int sectores_filesystem:32;
    unsigned char int13; 
    unsigned char nu; 
    unsigned char ebs; 
    char volume_id[4]; 
    char volume_label[11]; 
    char fs_type[8]; 
    char boot_code[448]; 
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct {
	unsigned char filename[8];
	unsigned char extension[3];
	unsigned char flags;
	unsigned char reservado;
	unsigned char createdTime;
	unsigned char createdHour[2];
	unsigned char createdDay[2];
	unsigned char accessedDay[2];
	unsigned char highBytesOfFirstClusterAddress[2];
	unsigned char writenTime[2];
	unsigned char writenDay[2];
	unsigned short fat_idx;
	unsigned char filesize[4];

} __attribute((packed)) Fat12Entry;


   void tipoDeArchivo(Fat12Entry *entry){
        if(entry->flags == 0x01){
                printf("Archivo solo de lectura \n");
        }else if (entry->flags == 0x02){
                printf("Archivo oculto \n");
        }else if(entry->flags == 0x04){
                printf("Archivo del sistema \n");  
        }else if(entry->flags == 0x10){
                printf("Es un directorio \n");   
        }
    }

void mostrarContenidoArchivo(FILE* in, Fat12Entry* entry, int tamanioEntry, Fat12BootSector* bs)
{
   unsigned int inicioFat = sizeof(Fat12BootSector) + (bs->sectores_reservados - 1) * bs->sector_size;
   unsigned int inicioRoot = inicioFat + bs->tamanio_fat * bs->cantidad_tablas_fats * bs->sector_size;
   unsigned int inicioData = inicioRoot + (bs->root_entries * tamanioEntry); 

   unsigned char buffer[bs->sector_size]; 
   unsigned int tamanioCluster = bs->sectores_por_cluster * bs->sector_size;
   unsigned int bloque = inicioData + (tamanioCluster * (entry->fat_idx - 2) ); 
   fseek(in, bloque, SEEK_SET); 
   fread(buffer, 1, bs->sector_size, in); 

   printf("Bloque [0x%X] :", bloque);
   printf("%s \n", buffer);

}

void print_file_info(FILE* in, Fat12Entry *entry, int tamanioEntry, Fat12BootSector *bs, int posicion) {

	switch (entry->filename[0]) {
	case 0x00:
		return; // unused entry
	case 0xE5:
                printf("\n");
	        printf("Deleted file: [?%.7s.%.3s] ", entry->filename + 1, entry->extension);
		break;
	case 0x05:
                printf("\n");
		printf("File starting with 0xE5: [%c%.7s.%.3s] ", 0xE5, entry->filename + 1, entry->extension);
                break;
	case 0x2E:
                printf("\n");
		printf("Directory: [%.8s.%.3s] ", entry->filename, entry->extension);
		break;
	default: 
                printf("\n");
                printf("File: [%.8s.%.3s] \n", entry->filename, entry->extension);
                printf("Tamaño de archivo [%i] bytes \n", (int)entry->filesize[0]);
	}

    tipoDeArchivo(entry);
    mostrarContenidoArchivo(in, entry, tamanioEntry, bs);
}

int main() {
	FILE * in = fopen("test.img", "rb");
	int i;
	PartitionTable pt[4];
	Fat12BootSector bs;
	Fat12Entry entry;

	fseek(in, 0x1BE, SEEK_SET); //Ir al inicio de la tabla de particiones
	fread(pt, sizeof(PartitionTable), 4, in); //Lectura

	for (i = 0; i < 4; i++) {
		if (pt[i].partition_type == 1) {
			printf("Encontrada particion FAT12 %d\n", i);
			break;
		}
	}

	if (i == 4) {
		printf("No encontrado filesystem FAT12, saliendo...\n");
		return -1;
	}

	fseek(in, 0, SEEK_SET);
	fread(&bs, sizeof(Fat12BootSector), 1, in);

	printf("En  0x%lx, sector size %d, FAT size %d sectors, %d FATs\n\n",
		ftell(in), bs.sector_size, bs.tamanio_fat, bs.cantidad_tablas_fats);

	fseek(in, (bs.sectores_reservados - 1 + bs.tamanio_fat * bs.cantidad_tablas_fats) *
		bs.sector_size, SEEK_CUR);

	printf("Root dir_entries %d \n", bs.root_entries);
	for (i = 0; i < bs.root_entries; i++) {
                //printf("\nAhora en 0x%lX\n", ftell(in));
		fread(&entry, sizeof(entry), 1, in);
                unsigned int ultimoSectorLeido = ftell(in);
		print_file_info(in, &entry, sizeof(entry), &bs, i);
                fseek(in, ultimoSectorLeido, SEEK_SET);
	}

	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
	fclose(in);
	return 0;
}
