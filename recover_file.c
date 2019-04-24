typedef struct
{
	unsigned char first_byte;
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	unsigned short starting_cluster;
	unsigned int file_size;
} __attribute((packed)) PartitionTable;

typedef struct
{
	unsigned char jmp[3];
	unsigned char oem[8];
	unsigned short sector_size;
	unsigned char sectors_by_cluster;
	unsigned short reserved_sectors;
	unsigned char fat_table_count;
	unsigned short root_entries;
	unsigned short sector_count;
	unsigned char media_descriptor;
	unsigned short fat_size;
	unsigned short sectors_by_track;
	unsigned short head_count;
	unsigned int hidden_sectors : 32;
	unsigned int filesystem_sectors : 32;
	unsigned char int13;
	unsigned char nu;
	unsigned char ebs;
	char volume_id[4];
	char volume_label[11];
	char fs_type[8];
	char boot_code[448];
	unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct
{
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
	unsigned char filesize[4];
} __attribute((packed)) Fat12Entry;

int main(int argc, char *argv[]) {

	int i;
	// Abrir FS
	in = fopen("test.img", "rb");

	Fat12BootSector bs;
	Fat12Entry entry;
	PartitionTable pt[4];
	char archivo[] = argv[2]; 

	// Ir al inicio de la tabla de particiones
	fseek(in, 0x1BE, SEEK_SET);
	fread(pt, sizeof(PartitionTable), 4, in);

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

	// Ir al offset cero para leer boot sector
	fseek(in, 0, SEEK_SET);
	fread(&bs, sizeof(Fat12BootSector), 1, in);

    // Ir al comienzo del directorio root
	fseek(in, (bs.reserved_sectors - 1 + bs.fat_size * bs.fat_table_count) * bs.sector_size, SEEK_CUR);
	
    printf("\nInit Root directory en 0x%lX\n", ftell(in));

	printf("Root dir_entries %d \n", bs.root_entries);

	for (i = 0; i < bs.root_entries; i++) {

		fread(&entry, sizeof(entry), 1, in);
        unsigned int ultimoSectorLeido = ftell(in);
		if(&entry->filename[0] == "0xE5"){ // "0xE5" -> "0x05"
			
		}
        fseek(in, ultimoSectorLeido, SEEK_SET);
	}

    printf("\n");
	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
	fclose(in);
	return 0;
}