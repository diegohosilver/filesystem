#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	unsigned char first_byte;
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	char start_sector[4];
	char length_sectors[4];
} __attribute((packed)) PartitionTable;

/**
 * Printear por pantalla valores hexadecimales
 */
void string_in_hex(void *in_string, int in_string_size)
{
	int i;
	int k = 0;

	for (i = 0; i < in_string_size; i++)
	{
		printf("%02x ", ((char *)in_string)[i] & 0xFF);
		k = k + 1;
		if (k == 16)
		{
			printf("\n");
			k = 0;
		}
	}
	printf("\n");
}

int main()
{
	FILE *in = fopen("test.img", "rb");
	int i;
	PartitionTable pt;

	// Ir al inicio de la tabla de particiones
	fseek(in, 0x1BE, SEEK_SET);
	// Leo primer entrada
	fread(&pt, sizeof(PartitionTable), 1, in);

	printf(" boot flag %02X\n", pt.first_byte);
	printf(" chs start ");
	string_in_hex(pt.start_chs, 3);
	printf(" partition type %02X\n", pt.partition_type);
	printf(" chs end ");
	string_in_hex(pt.end_chs, 3);
	printf(" start sector ");
	string_in_hex(pt.start_sector, 4);
	printf(" length sectors ");
	string_in_hex(pt.length_sectors, 4);
}