#include "../../include/json/cjson_ticket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/json/cjson_utils.h"
#include "../../include/ticket/ticket.h"
#include "../../include/ticket/ticket_list.h"

// Variabile globale per mantenere il riferimento al file aperto
static FILE *ticket_file = NULL;

// Dichiarazione dei prototipi
static cJSON *parse_ticket_list_to_array_json(TicketList *ticket_list);
static TicketList *parse_array_json_to_ticket_list(cJSON *json_array);


TicketList *load_ticket_list(const char *mode) {
  char *file_buffer = NULL;
  cJSON *json_array = NULL;
  TicketList *ticket_list = NULL;
  // Apertura del file JSON con lock
  ticket_file = open_file_with_lock(TICKETS_FILE, mode);
  if (ticket_file == NULL) {
    return NULL;
  }

  // Lettura del contenuto
  file_buffer = parse_file_json(ticket_file);
  if (file_buffer == NULL) {
    fclose(ticket_file);
    ticket_file = NULL;
    return NULL;
  }

  // Parsing dell'array JSON
  json_array = parse_string_to_array_json(file_buffer);
  if (json_array == NULL) {
    fprintf(stderr, "parse_string_to_array_json: tickets.json\n");
    free(file_buffer);
    fclose(ticket_file);
    ticket_file = NULL;
    return NULL;
  }

  // Liberiamo il buffer dopo il parsing
  free(file_buffer);

  // Costruzione della ticket_list
  ticket_list = parse_array_json_to_ticket_list(json_array);

  // Pulizia
  cJSON_Delete(json_array);

  // Gestione chiusura file in base al mode
  if (strcmp(mode, "r") == 0) {
    fclose(ticket_file);
    ticket_file = NULL;
  }

  return ticket_list;
}

int save_ticket_list(TicketList *ticket_list) {
  // Creazione dell'array JSON
  cJSON *json_array = parse_ticket_list_to_array_json(ticket_list);
  if (json_array == NULL) {
    fclose(ticket_file);
    ticket_file = NULL;
    return -1;
  }
  // Scrittura su file e chiusura
  int outcome = parse_json_array_to_file(json_array, TICKETS_FILE);
  cJSON_Delete(json_array);

  if (ticket_file != NULL) {
    fclose(ticket_file);
    ticket_file = NULL; // Importante: resetta il puntatore globale
  }
  return outcome;
}

// ======================== Funzioni di supporto static =========================

// Conversione di una TicketList in un array JSON
static cJSON *parse_ticket_list_to_array_json(TicketList *ticket_list) {
  // Creazione dell'array JSON
  cJSON *json_array = cJSON_CreateArray();
  if (json_array == NULL) {
    fprintf(stderr, "Errore: fallita creazione cJSON array.\n");
    return NULL;
  }

  // Iterazione su ogni ticket nella lista
  int size = ticket_list->count;
  for (int i = 0; i < size; i++) {
    Ticket *current_ticket = get_by_index(ticket_list, i);
    if (!current_ticket)
      continue;

    // Creazione dell'oggetto JSON corrispondente al ticket
    cJSON *json_ticket = cJSON_CreateObject();
    if (json_ticket == NULL) {
      fprintf(stderr, "Errore: fallita creazione cJSON object.\n");
      cJSON_Delete(json_array);
      return NULL;
    }

    // Aggiunta dei campi del ticket all'oggetto JSON
    cJSON_AddStringToObject(json_ticket, "title", current_ticket->title);
    cJSON_AddStringToObject(json_ticket, "description", current_ticket->description);
    cJSON_AddStringToObject(json_ticket, "date", current_ticket->date);
    cJSON_AddNumberToObject(json_ticket, "id", current_ticket->id);
    cJSON_AddNumberToObject(json_ticket, "priority", current_ticket->priority);
    cJSON_AddNumberToObject(json_ticket, "status", current_ticket->status);
    cJSON_AddNumberToObject(json_ticket, "agentId", current_ticket->agentId);
    cJSON_AddNumberToObject(json_ticket, "userId", current_ticket->userId);

    // Aggiunta dell'oggetto ticket all'array JSON
    cJSON_AddItemToArray(json_array, json_ticket);
  }
  return json_array;
}

// Conversione di un array JSON in una TicketList
static TicketList *parse_array_json_to_ticket_list(cJSON *json_array) {
  TicketList *ticket_list = ticket_list_init();
  if (!ticket_list) {
    fprintf(stderr, "ticket_list_init failed\n");
    return NULL;
  }

  // Variabile iteratore per la macro
  cJSON *cjson_ticket = NULL;

  // Iterazione su ogni elemento dell'array JSON
  cJSON_ArrayForEach(cjson_ticket, json_array) {

    // Estrazione campi
    cJSON *j_title = cJSON_GetObjectItem(cjson_ticket, "title");
    cJSON *j_desc = cJSON_GetObjectItem(cjson_ticket, "description");
    cJSON *j_date = cJSON_GetObjectItem(cjson_ticket, "date");
    cJSON *j_id = cJSON_GetObjectItem(cjson_ticket, "id");
    cJSON *j_priority = cJSON_GetObjectItem(cjson_ticket, "priority");
    cJSON *j_status = cJSON_GetObjectItem(cjson_ticket, "status");
    cJSON *j_agentId = cJSON_GetObjectItem(cjson_ticket, "agentId");
    cJSON *j_userId = cJSON_GetObjectItem(cjson_ticket, "userId");

    // Validazione campi obbligatori del Ticket nel JSON
    if (!j_title    || !cJSON_IsString(j_title)    ||
        !j_desc     || !cJSON_IsString(j_desc)     ||
        !j_date     || !cJSON_IsString(j_date)     ||
        !j_id       || !cJSON_IsNumber(j_id)       ||
        !j_priority || !cJSON_IsNumber(j_priority) ||
        !j_status   || !cJSON_IsNumber(j_status)   ||
        !j_agentId  || !cJSON_IsNumber(j_agentId)  ||
        !j_userId   || !cJSON_IsNumber(j_userId)) {
        
        fprintf(stderr, "Errore: ticket nel JSON con campi mancanti o errati.\n");
        continue;
    }
    // Caricamento dei dati nel modello Ticket
    Ticket *current_ticket = load_ticket_data(
        j_title->valuestring,
        j_desc->valuestring,
        j_date->valuestring,
        (unsigned int)j_id->valueint,
        j_priority->valueint,
        j_status->valueint,
        (unsigned int)j_agentId->valueint,
        (unsigned int)j_userId->valueint
    );
    if (!current_ticket) {
      fprintf(stderr, "Errore: fallito caricamento ticket dal JSON.\n");
      clear_ticket_list(ticket_list); // Pulisce tutto se fallisce malloc
      return NULL;
    }
    // Aggiunta alla lista
    add_ticket(ticket_list, current_ticket);
  }
  return ticket_list;
}