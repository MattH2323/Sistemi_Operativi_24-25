#ifndef CJSON_H
#define CJSON_H

#include <stdio.h>

#include "cJSON.h"

// Massima lunghezza del nome file
#define FILE_NAME_MAX_LENGTH 32

// Apri un file con lock in modalità specificata ("r", "w", "a", ecc.)
FILE *open_file_with_lock(const char *file_name, const char *mode);

// Legge il contenuto di un file e restituisce un buffer di stringa (allocato internamente).
char *parse_file_json(FILE *file);

// Converte una stringa JSON in un array cJSON
cJSON *parse_string_to_array_json(char *file_buffer);

// Scrive un array cJSON su file. Restituisce 0 o -1 in caso di errore.
int parse_json_array_to_file(cJSON *json_array, char *file_name);

#endif // CJSON_H