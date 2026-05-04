#include "../../include/json/cjson_user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/json/cjson_utils.h"
#include "../../include/user/user_list.h"

// Variabile globale per mantenere il riferimento al file aperto
static FILE *user_file = NULL;

// Dichiarazione dei prototipi
static cJSON *parse_user_list_to_array_json(UserList *user_list);
static UserList *parse_array_json_to_user_list(cJSON *json_array);


UserList *load_user_list(const char *mode) {
  char *file_buffer;
  cJSON *json_array ;
  UserList *user_list;

  // Apertura del file JSON
  user_file = open_file_with_lock(USERS_FILE, mode);
  if (user_file == NULL) {
    return NULL;
  }

  // Lettura del contenuto
  file_buffer = parse_file_json(user_file);
  if (file_buffer == NULL) {
    fclose(user_file);
    return NULL;
  }

  // Parsing dell'array JSON
  json_array = parse_string_to_array_json(file_buffer);
  if (json_array == NULL) {
    fprintf(stderr, "parse_array_json: users.json\n");
    free(file_buffer);
    fclose(user_file);
    return NULL;
  }

  // Liberiamo il buffer dopo il parsing
  free(file_buffer);

  // Costruzione della user_list
  user_list = parse_array_json_to_user_list(json_array);

  // Pulizia
  cJSON_Delete(json_array);

  // Gestione chiusura file in base al mode
  if (strcmp(mode, "r") == 0) {
    fclose(user_file);
    user_file = NULL;
  }

  return user_list;
}

int save_user_list(UserList *user_list) {
  // Creazione dell'array JSON
  cJSON *json_array = parse_user_list_to_array_json(user_list);
  if (json_array == NULL) {
    fclose(user_file);
    user_file = NULL;
    return -1;
  }

  // scrittura su file e chiusura
  int outcome = parse_json_array_to_file(json_array, USERS_FILE);
  cJSON_Delete(json_array);
  if (user_file != NULL) {
    fclose(user_file);
    user_file = NULL; // Resetta il puntatore globale
  }
  return outcome;
}

// ======================== Funzioni di supporto static =========================

// Funzione di supporto per convertire UserList in cJSON array
static cJSON *parse_user_list_to_array_json(UserList *user_list) {
  cJSON *json_array = cJSON_CreateArray();
  if (json_array == NULL) {
    fprintf(stderr, "Errore: fallita creazione cJSON array.\n");
    return NULL;
  }

  // Itera su ogni utente nella UserList e crea un oggetto JSON per ciascuno
  for (int i = 0; i < user_list->count; i++) {
    User *current_user = &(user_list->users[i]);
    cJSON *json_user = cJSON_CreateObject();
    if (json_user == NULL) {
      fprintf(stderr, "Errore: fallita creazione cJSON object.\n");
      cJSON_Delete(json_array);
      return NULL;
    }
    cJSON_AddStringToObject(json_user, "username", current_user->username);
    cJSON_AddStringToObject(json_user, "password", current_user->password);
    cJSON_AddNumberToObject(json_user, "user_id", current_user->user_id);
    cJSON_AddStringToObject(json_user, "email", current_user->email);
    cJSON_AddNumberToObject(json_user, "type", current_user->type);
    cJSON_AddItemToArray(json_array, json_user);
  }
  return json_array;
}

// Funzione di supporto per convertire cJSON array in UserList
static UserList *parse_array_json_to_user_list(cJSON *json_array) {
  UserList *user_list = NULL;

  // Alloca la struttura principale (UserList)
  user_list = user_list_init();
  if (!user_list) {
    fprintf(stderr, "user_list_init failed\n");
    return NULL;
  }

  // Variabili temporanee per i dati letti
  int array_size = cJSON_GetArraySize(json_array);

  // Itera su ogni oggetto utente nell'array JSON
  for (int i = 0; i < array_size; i++) {
    // Estrazione dell'oggetto utente
    cJSON *cjson_user = cJSON_GetArrayItem(json_array, i);
    if (!cjson_user) {
      continue;
    }
    cJSON *j_username = cJSON_GetObjectItem(cjson_user, "username");
    cJSON *j_password = cJSON_GetObjectItem(cjson_user, "password");
    cJSON *j_email = cJSON_GetObjectItem(cjson_user, "email");
    cJSON *j_id = cJSON_GetObjectItem(cjson_user, "user_id");
    cJSON *j_type = cJSON_GetObjectItem(cjson_user, "type");

    // Verifica che tutti i campi esistano e siano del tipo corretto
    if ((!j_username || !cJSON_IsString(j_username)) ||
        (!j_password || !cJSON_IsString(j_password)) ||
        (!j_email || !cJSON_IsString(j_email)) ||
        (!j_id || !cJSON_IsNumber(j_id)) ||
        (!j_type || !cJSON_IsNumber(j_type))) {

      fprintf(stderr,
              "Errore: utente nel file JSON con campi mancanti o errati.\n");
      continue;
    }

    // Controlla se è necessario ridimensionare l'array
    if (user_list->count >= user_list->capacity) {
      if (grow_user_list(user_list) == NULL) {
        clear_user_list(user_list);
        return NULL;
      }
    }

    User *current_user = &(user_list->users[user_list->count]);

    // Popolamento dei campi
    strncpy(current_user->username, j_username->valuestring,
            sizeof(current_user->username) - 1);
    current_user->username[sizeof(current_user->username) - 1] = '\0';

    strncpy(current_user->password, j_password->valuestring,
            sizeof(current_user->password) - 1);
    current_user->password[sizeof(current_user->password) - 1] = '\0';

    strncpy(current_user->email, j_email->valuestring,
            sizeof(current_user->email) - 1);
    current_user->email[sizeof(current_user->email) - 1] = '\0';

    current_user->user_id = (unsigned int)j_id->valuedouble;
    current_user->type = j_type->valueint;

    // Incrementa il contatore degli utenti
    user_list->count++;
  }

  return user_list;
}
