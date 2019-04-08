#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char start_sector[4];
    char length_sectors[4];
} __attribute((packed)) PartitionTable;

void string_in_hex(void *in_string, int in_string_size)
{
        int i;
        int k = 0;

        for (i = 0; i < in_string_size; i++)
        {
                printf("%02x ", ((char *)in_string)[i]& 0xFF);
                k = k + 1;
                if (k == 16)
                {
                        printf("\n");
                        k = 0;
                }
        }
        printf("\n");
}

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[1];

    fseek(in, 0x1BE, SEEK_SET); // Ir al inicio de la tabla de particiones
    fread(pt, sizeof(PartitionTable), 1, in); // leo primer entrada

    printf(" boot flag %02X\n", pt[0].first_byte);
    printf(" chs start ");
    string_in_hex(pt[0].start_chs,3);
    printf(" partition type %02X\n", pt[0].partition_type);
    printf(" chs end ");
    string_in_hex(pt[0].end_chs,3);
    printf(" start sector ");
    string_in_hex(pt[0].start_sector, 4);
    printf(" length sectors ");
    string_in_hex(pt[0].length_sectors, 4);
}