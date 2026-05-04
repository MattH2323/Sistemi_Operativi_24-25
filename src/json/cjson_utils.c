
#include "../../include/json/cjson_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#include "../../include/json/cJSON.h"

// Apertura del file e acquisizione del lock
FILE *open_file_with_lock(const char *file_name, const char *mode) {
  FILE *file_ptr = fopen(file_name, mode);

  // Controllo apertura file
  if (file_ptr == NULL) {
    return NULL;
  } else {
    // Stabilire il tipo di lock
    int lock_type;
    if (strchr(mode, 'w') || strchr(mode, '+') || strchr(mode, 'a')) {
      lock_type = LOCK_EX; // Scrittura
    } else {
      lock_type = LOCK_SH; // Lettura
    }
    // Recupero del file descriptor
    int file_descriptor = fileno(file_ptr);
    if (flock(file_descriptor, lock_type) == -1) {
      perror("Errore nell'acquisizione del lock");
      fclose(file_ptr);
      return NULL;
    }
  }
  return file_ptr;
}

// Lettura del file JSON in un buffer allocato
char *parse_file_json(FILE *file) {
  long file_length;
  char *file_buffer = NULL;

  // Controllo di validità del puntatore (Dovrebbe essere già fatto nel
  // chiamante)
  if (file == NULL) {
    fprintf(stderr, "Errore: Puntatore a file NULL in parse_file_json.\n");
    return NULL;
  }
  // Determina la lunghezza del file
  if (fseek(file, 0, SEEK_END) != 0) {
    perror("Errore in fseek (SEEK_END)");
    return NULL;
  }
  file_length = ftell(file);
  if (file_length == -1) {
    perror("Errore in ftell");
    return NULL;
  }
  // Riporta il puntatore all'inizio del file
  if (fseek(file, 0, SEEK_SET) != 0) {
    perror("Errore in fseek (SEEK_SET)");
    return NULL;
  }

  // Gestione del file vuoto
  if (file_length == 0) {
    // Se il file è vuoto, restituiamo una stringa vuota ma allocata ('\0')
    file_buffer = (char *)malloc(1);
    if (file_buffer == NULL) {
      perror("Errore in malloc per buffer vuoto");
    } else {
      file_buffer[0] = '\0';
    }
    return file_buffer;
  }

  // Alloca un buffer (dimensione + 1 per il terminatore)
  file_buffer = (char *)malloc(file_length + 1);
  if (file_buffer == NULL) {
    perror("Errore in malloc");
    return NULL;
  }

  // Leggi il contenuto e termina la stringa
  size_t read_bytes = fread(file_buffer, 1, file_length, file);
  if (read_bytes != (size_t)file_length) {
    fprintf(stderr, "Errore di lettura del file\n");
    free(file_buffer);
    return NULL;
  }
  file_buffer[file_length] = '\0';
  return file_buffer;
}

// Scrittura dell'array JSON su file
int parse_json_array_to_file(cJSON *json_array, char *file_name) {
  char *json_string = NULL;
  char tmp_file_name[FILE_NAME_MAX_LENGTH];
  FILE *file_tmp = NULL;

  // Conversione JSON -> Stringa
  json_string = cJSON_Print(json_array);
  if (json_string == NULL) {
    return -1;
  }

  // Creazione nome file temporaneo
  snprintf(tmp_file_name, sizeof(tmp_file_name), "%s.tmp", file_name);

  // Apertura file temporaneo
  file_tmp = fopen(tmp_file_name, "w");
  if (file_tmp == NULL) {
    perror("Impossibile aprire il file temporaneo");
    free(json_string);
    return -1;
  }

  // Scrittura stringa
  if (fputs(json_string, file_tmp) < 0) {
    fprintf(stderr, "Errore durante la scrittura dei dati.\n");
    free(json_string);
    fclose(file_tmp);
    remove(tmp_file_name);
    return -1;
  }

  // Pulizia memoria
  free(json_string);

  // Chiusura e flush
  if (fclose(file_tmp) != 0) {
    perror("Errore nel salvataggio");
    remove(tmp_file_name);
    return -1;
  }

  // Rinominazione finale
  if (rename(tmp_file_name, file_name) != 0) {
    perror("Errore nel rinominare il file finale");
    remove(tmp_file_name);
    return -1;
  }
  return 0;
}

// Parsing da stringa ad array JSON
cJSON *parse_string_to_array_json(char *file_buffer) {
  cJSON *json_list = NULL;

  // File Vuoto
  if (file_buffer[0] == '\0') {
    return cJSON_CreateArray();
  }

  // Parsing del contenuto
  json_list = cJSON_Parse(file_buffer);

  // Errore nel parsing
  if (json_list == NULL) {
    fprintf(stderr, "Errore durante il parsing del JSON nel file\n");
    return NULL;
  }

  // controllo formato
  if (!cJSON_IsArray(json_list)) {
    fprintf(stderr, "Il JSON nel file non è un array. Operazione annullata.\n");
    cJSON_Delete(json_list);
    return NULL;
  }
  return json_list;
}
