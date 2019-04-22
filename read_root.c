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
    char oem[8];
    unsigned short sector_size;
	char bytes_per_logical_sector;
    unsigned short reserved_sectors;
    char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short logical_sectors;
    char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short heads_number;
    char hidden_sectors[4];
    char total_logical_sectors[4];
    char drive_number;
    char reserved;
    char extended_boot_signature;
    char volume_id[4];
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct {
	unsigned char filename[8];
    unsigned char extension[3];
	char flags;
	char reserved;
	char create_time_tenths_seconds;
	unsigned short create_time;
    unsigned short create_day;
	unsigned short last_access_date;
	unsigned short ignore_in_fat12;
	unsigned short last_write_time;
	unsigned short last_write_date;
	unsigned short fat_idx;
	char filesize[4];
} __attribute((packed)) Fat12Entry;

void print_file_info(Fat12Entry *entry) {
    switch(entry->filename[0]) {
    case 0x00:
        return; // unused entry
    case 0xE5:
        printf("Archivo borrado: [?%.7s.%.3s]\n", entry->filename, entry->extension); // COMPLETAR
        return;
    case 0x05:
        printf("Archivo que comienza con 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename, entry->extension); // COMPLETAR 
        break;
    case 0x2E:
        printf("Directorio: [%.8s.%.3s]\n", entry->filename, entry->extension); // COMPLETAR 
        break;
    default:
        printf("Archivo: [%.8s.%.3s]\n", entry->filename, entry->extension); // COMPLETAR 
    }
    
}

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
   
	fseek(in, 0x1BE, SEEK_SET);
   
    for(i=0; i<4; i++) {   
        fread(pt, sizeof(PartitionTable), 4, in);             
        if(pt[i].partition_type == 1) {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }
    
    fseek(in, 0, SEEK_SET);
	fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs\n\n", 
           ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
           
    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    printf("Root dir_entries %d \n", bs.root_dir_entries);
    for(i=0; i<bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry);
    }
    
    printf("\nLeido Root directory, ahora en 0x%X\n", ftell(in));
    fclose(in);
    return 0;
}
