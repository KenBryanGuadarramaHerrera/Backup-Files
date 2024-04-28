#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

void remove_directory(const char *path);
void create_backup_directory(const char *backup_directory);

int main(int argc, char *argv[]) {
    // Verificar que se haya proporcionado el directorio destino como argumento
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directorio_destino>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Directorio fuente y directorio destino
    char *source_dir = "dirfuente";
    char *backup_dir = argv[1];

    // Eliminar y crear el directorio de respaldo
    remove_directory(backup_dir);
    create_backup_directory(backup_dir);

    DIR *dir;
    struct dirent *entry;
    int total_files = 0;

    // Abrir el directorio fuente
    if ((dir = opendir(source_dir)) == NULL) {
        perror("Error al abrir el directorio fuente");
        exit(EXIT_FAILURE);
    }

    // Crear el archivo de lista de archivos en el directorio de respaldo
    char file_list_path[BUFFER_SIZE];
    sprintf(file_list_path, "%s/files_list.txt", backup_dir);
    FILE *file_list = fopen(file_list_path, "w");
    if (file_list == NULL) {
        perror("Error al crear el archivo de lista de archivos");
        exit(EXIT_FAILURE);
    }

    // Leer los archivos del directorio fuente y escribirlos en el archivo de lista de archivos
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Solo archivos regulares
            fprintf(file_list, "%s\n", entry->d_name);
            total_files++;
        }
    }
    closedir(dir);
    fprintf(file_list, "Total de archivos: %d\n", total_files);
    fclose(file_list);

    // Respaldar archivos: Copiar cada archivo del directorio fuente al directorio de respaldo
    if ((dir = opendir(source_dir)) == NULL) {
        perror("Error al abrir el directorio fuente");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Solo archivos regulares
            // Construir las rutas completas para copiar
            char src_path[BUFFER_SIZE], dest_path[BUFFER_SIZE];
            snprintf(src_path, BUFFER_SIZE, "%s/%s", source_dir, entry->d_name);
            snprintf(dest_path, BUFFER_SIZE, "%s/%s", backup_dir, entry->d_name);
            // Copiar el archivo
            FILE *src = fopen(src_path, "rb");
            FILE *dest = fopen(dest_path, "wb");
            if (src != NULL && dest != NULL) {
                char ch;
                while (fread(&ch, sizeof(char), 1, src) == 1)
                    fwrite(&ch, sizeof(char), 1, dest);
                fclose(src);
                fclose(dest);
                printf("Archivo \"%s\" respaldado con éxito.\n", entry->d_name);
            } else {
                perror("Error al abrir archivos para respaldo");
            }
        }
    }
    closedir(dir);

    printf("Todos los archivos han sido respaldados correctamente.\n");

    return 0;
}

// Función para eliminar un directorio y todos sus contenidos
void remove_directory(const char *path) {
    DIR *dir = opendir(path);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char full_path[BUFFER_SIZE];
                sprintf(full_path, "%s/%s", path, entry->d_name);
                if (entry->d_type == DT_DIR) {
                    remove_directory(full_path);
                } else {
                    unlink(full_path);
                }
            }
        }
        closedir(dir);
        rmdir(path);
    }
}

// Función para crear un directorio de respaldo
void create_backup_directory(const char *backup_directory) {
    if (mkdir(backup_directory, 0777) < 0) {
        perror("Error al crear el directorio de respaldo");
        exit(EXIT_FAILURE);
    }
}
