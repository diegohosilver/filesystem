#include <stdio.h>
#include <stdlib.h>

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
	unsigned char flags;
	unsigned char reserved;
	unsigned char created_time;
	unsigned char created_hour[2];
	unsigned char created_day[2];
	unsigned char accessed_day[2];
	unsigned char high_bytes_first_cluster_address[2];
	unsigned char writen_time[2];
	unsigned char writen_day[2];
	unsigned short fat_idx;
	unsigned char filesize[4];
} __attribute((packed)) Fat12Entry;

void print_file_type(Fat12Entry *entry)
{
	switch (entry->flags)
	{
	case 0x01:
		printf("Flag: Archivo de solo lectura \n");
		break;
	case 0x02:
		printf("Flag: Archivo oculto \n");
		break;
	case 0x04:
		printf("Flag: Archivo del sistema \n");
		break;
	case 0x10:
		printf("Flag: Directorio \n");
	}
}

void print_file_content(FILE *in, Fat12Entry *entry, int entry_size, Fat12BootSector *bs)
{
	unsigned int fat_start = sizeof(Fat12BootSector) + (bs->reserved_sectors - 1) * bs->sector_size;
	unsigned int root_start = fat_start + bs->fat_size * bs->fat_table_count * bs->sector_size;
	unsigned int data_start = root_start + (bs->root_entries * entry_size);

	unsigned char buffer[bs->sector_size];
	unsigned int cluster_size = bs->sectors_by_cluster * bs->sector_size;
	unsigned int block = data_start + (cluster_size * (entry->fat_idx - 2));

	fseek(in, block, SEEK_SET);

	if (entry->flags == 0x10)
	{
		Fat12Entry sub_entry;

		for (int i = 0; i < bs->root_entries; i++)
		{
			fread(&sub_entry, sizeof(sub_entry), 1, in);

			unsigned int last_read_sector = ftell(in);

			print_file_info_only(in, &sub_entry, sizeof(sub_entry), bs);

			fseek(in, last_read_sector, SEEK_SET);
		}
	}
	else
	{
		fread(buffer, 1, bs->sector_size, in);
	}

	printf("Bloque [0x%X] :", block);
	printf("%s \n", buffer);
}

void print_file_info(FILE *in, Fat12Entry *entry, int entry_size, Fat12BootSector *bs)
{

	switch (entry->filename[0])
	{
	case 0x00:
		return; // unused entry
	case 0xE5:
		printf("\n");
		printf("Archivo eliminado: [?%.7s.%.3s] ", entry->filename + 1, entry->extension);
		break;
	case 0x05:
		printf("\n");
		printf("Archivo comienza con 0xE5: [%c%.7s.%.3s] ", 0xE5, entry->filename + 1, entry->extension);
		break;
	case 0x2E:
		printf("\n");
		printf("Directorio: [%.8s.%.3s] ", entry->filename, entry->extension);
		break;
	default:
		printf("\n");
		printf("File: [%.8s.%.3s] \n", entry->filename, entry->extension);
		printf("Tamaño de archivo [%i] bytes \n", (int)entry->filesize[0]);
	}

	print_file_type(entry);
	print_file_content(in, entry, entry_size, bs);
}

void print_file_info_only(FILE *in, Fat12Entry *entry, int entry_size, Fat12BootSector *bs)
{

	switch (entry->filename[0])
	{
	case 0x00:
		return; // unused entry
	case 0xE5:
		printf("\n");
		printf("Archivo eliminado: [?%.7s.%.3s] ", entry->filename + 1, entry->extension);
		break;
	case 0x05:
		printf("\n");
		printf("Archivo comienza con 0xE5: [%c%.7s.%.3s] ", 0xE5, entry->filename + 1, entry->extension);
		break;
	case 0x2E:
		printf("\n");
		printf("Directorio: [%.8s.%.3s] ", entry->filename, entry->extension);
		break;
	default:
		printf("\n");
		printf("File: [%.8s.%.3s] \n", entry->filename, entry->extension);
		printf("Tamaño de archivo [%i] bytes \n", (int)entry->filesize[0]);
	}
}

int main()
{
	FILE *in = fopen("test.img", "rb");
	int i;
	PartitionTable pt[4];
	Fat12BootSector bs;
	Fat12Entry entry;

	fseek(in, 0x1BE, SEEK_SET);				  //Ir al inicio de la tabla de particiones
	fread(pt, sizeof(PartitionTable), 4, in); //Lectura

	for (i = 0; i < 4; i++)
	{
		if (pt[i].partition_type == 1)
		{
			printf("Encontrada particion FAT12 %d\n", i);
			break;
		}
	}

	if (i == 4)
	{
		printf("No encontrado filesystem FAT12, saliendo...\n");
		return -1;
	}

	fseek(in, 0, SEEK_SET);
	fread(&bs, sizeof(Fat12BootSector), 1, in);

	printf("En  0x%lx, sector size %d, FAT size %d sectors, %d FATs\n\n", ftell(in), bs.sector_size, bs.fat_size, bs.fat_table_count);

	fseek(in, (bs.reserved_sectors - 1 + bs.fat_size * bs.fat_table_count) * bs.sector_size, SEEK_CUR);

	printf("Root dir_entries %d \n", bs.root_entries);

	for (i = 0; i < bs.root_entries; i++)
	{

		fread(&entry, sizeof(entry), 1, in);

		unsigned int last_read_sector = ftell(in);

		print_file_info(in, &entry, sizeof(entry), &bs);

		fseek(in, last_read_sector, SEEK_SET);
	}

	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));

	fclose(in);

	return 0;
}
