#include <stdio.h>
#include <string.h>
#include <dirent.h>

void list_recursively(char *path, int deep);

int main() {
    // Path para listar archivos
    char path[100];

    // Directorio elegido por el usuario
    printf("Ingrese un directorio para listar archivos: ");
    scanf("%s", path);

    list_recursively(path, -1);

    return 0;
}

void print_tree(int deep) {
    for (int i = 0; i < deep; i ++) {
        printf("%s", "│ ");
    }
}

void list_recursively(char *basePath, int deep) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Retornar si no se pudo abrir el directorio
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {

            // Incrementar el contador de subdirectorio
            deep ++;
            print_tree(deep);

            // Mostrar el nombre del archivo/directorio
            printf("├─%s\n", dp->d_name);

            // Construir un nuevo path a partir de nuestro path base
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            // Bajar un nivel más
            list_recursively(path, deep);

            // Decrementar el contador de subdirectorio
            deep --;
        }
    }

    closedir(dir);
}