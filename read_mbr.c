#include <stdio.h>
#include <stdlib.h>

int main()
{
	FILE *in = fopen("test.img", "rb");
	unsigned int i, start_sector, length_sectors;

	fseek(in, 0x1BE, SEEK_SET); // Voy al inicio...

	for (i = 0; i < 4; i++)
	{ 
		// Leo las entradas
		printf("\n");
		printf("Partition entry %d: First byte %02X\n", i, fgetc(in));
		printf("  Comienzo de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));

		int tipo_particion = fgetc(in);
		printf("  Partition type: ");

		if (tipo_particion == 0)
		{
			printf("Entrada de partición vacía\n");
		}
		else if (tipo_particion == 1)
		{
			printf("FAT12\n");
		}
		else
		{
			printf("Otro tipo de particion\n");
		}

		printf("  Fin de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));

		fread(&start_sector, 4, 1, in);
		fread(&length_sectors, 4, 1, in);
		
		printf("  Dirección LBA relativa 0x%08X, de tamaño en sectores %d\n", start_sector, length_sectors);
	}
	fclose(in);
	return 0;
}
