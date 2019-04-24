#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

unsigned int fat_start;
unsigned int root_start;
unsigned int data_start;
unsigned int cluster_size;
Fat12BootSector bs;
FILE *in;
Fat12Entry entry;
int stack[100];  // Máxima cantidad de subdirectorios -> TODO: aumentar de forma dinámica este valor
char search[11]; // Término a buscar

/**
 * Printear contenido del archivo 
 */
void print_file_content(Fat12Entry *entry)
{
    unsigned char buffer[bs.sector_size];
    unsigned int block = get_block_from_current_entry(entry);

    fseek(in, block, SEEK_SET);
    fread(buffer, 1, bs.sector_size, in);

    if (string_contains(entry->filename, search) == 1 || string_contains(buffer, search) == 1)
    {
        printf("Se encontró un resultado! Es un archivo \n");

        print_file_info(entry);

        printf("Bloque [0x%X] :", block);
        printf("%s \n", buffer);
    }
}

/**
 * Printear información sobre el archivo
 */
void print_file_info(Fat12Entry *entry)
{
    switch (entry->filename[0])
    {
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

/**
 * Devolver el bloque el cual tiene el contenido del entry actual
 */
int get_block_from_current_entry(Fat12Entry *entry)
{
    return data_start + (cluster_size * (entry->fat_idx - 2));
}

/**
 * Verificar si un string existe dentro de otro
 */
int string_contains(char *string, char *toFind)
{
    int slen = strlen(string);
    int tFlen = strlen(toFind);
    int found = 0;

    if (slen >= tFlen)
    {
        for (int s = 0, t = 0; s < slen; s++)
        {
            do
            {

                if (string[s] == toFind[t])
                {
                    if (++found == tFlen)
                        return 1;
                    s++;
                    t++;
                }
                else
                {
                    s -= found;
                    found = 0;
                    t = 0;
                }

            } while (found);
        }
        return 0;
    }
    else
        return -1;
}

/**
 * Recorrer recursivamente la estructura del root
 */
void traverseFat(int offset, int deep, int is_root)
{
    if (is_root == 1)
    {
        // Ir al comienzo del directorio root
        fseek(in, (bs.reserved_sectors - 1 + bs.fat_size * bs.fat_table_count) * bs.sector_size, SEEK_CUR);
    }
    else
    {
        // Ir al offset seleccionado
        fseek(in, offset, SEEK_SET);
    }

    // Leer entry
    fread(&entry, sizeof(entry), 1, in);

    // Leer hasta que nos encontremos con un entry vacío
    while (entry.filename[0] != 0x00)
    {
        // Verificar si el entry es un directorio o un archivo
        if (entry.flags == 0x10)
        {
            // Saltear el entry si es del tipo dot y leer el próximo
            if (strncmp(&entry.filename[0], ".", 1) != 0)
            {
                // Validar que tipo de información printear según el ingreso del usuario
                if (string_contains(entry.filename, search) == 1)
                {
                    printf("Se encontró un resultado! Es un directorio \n");
                }

                // Guardar el offset actual en el stack
                stack[deep] = ftell(in);

                // Incrementar contador de subdirectorios y recorrer el próximo
                deep++;
                traverseFat(get_block_from_current_entry(&entry), deep, 0);
                deep--;

                // Una vez terminado de leer el subdirectorio, leer el próximo entry
                fseek(in, stack[deep], SEEK_SET);
                fread(&entry, sizeof(entry), 1, in);
            }
            else
            {
                fseek(in, ftell(in), SEEK_SET);
                fread(&entry, sizeof(entry), 1, in);
            }
        }
        else
        {
            // Guardar el offset actual
            int current_offset = ftell(in);

            print_file_content(&entry);

            fseek(in, current_offset, SEEK_SET);
            fread(&entry, sizeof(entry), 1, in);
        }
    }
}

int main()
{
    int deep = 0;
    int i;
    PartitionTable pt[4];

    // Abrir FS
    in = fopen("test.img", "rb");

    // Ir al inicio de la tabla de particiones
    fseek(in, 0x1BE, SEEK_SET);
    fread(pt, sizeof(PartitionTable), 4, in);

    for (i = 0; i < 4; i++)
    {
        if (pt[i].partition_type == 1)
        {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }

    // Terminar ejecución si no existe fat12
    if (i == 4)
    {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }

    // Ir al offset cero para leer boot sector
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);

    // Calcular datos
    fat_start = sizeof(Fat12BootSector) + (bs.reserved_sectors - 1) * bs.sector_size;
    root_start = fat_start + bs.fat_size * bs.fat_table_count * bs.sector_size;
    data_start = root_start + (bs.root_entries * sizeof(entry));
    cluster_size = bs.sectors_by_cluster * bs.sector_size;

    // Leer ingreso del usuario y guardar
    printf("Ingrese el término a buscar: ");
    scanf("%s", search);
    printf("\n");

    // Ir al root directory
    traverseFat(0, deep, 1);

    printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));

    fclose(in);

    return 0;
}