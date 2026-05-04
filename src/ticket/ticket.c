#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/ticket/ticket.h"

Ticket *create_ticket(char *title, char *description, unsigned int userId) {
  // Allocazione della memoria
  Ticket *new_ticket = malloc(sizeof(Ticket));
  if (new_ticket == NULL) {
    perror("create_ticket: malloc failed");
    return NULL;
  }
  // Inizializzazione dei campi
  strncpy(new_ticket->title, title, TITLE_BUFFER_SIZE - 1);
  new_ticket->title[TITLE_BUFFER_SIZE - 1] = '\0';
  strncpy(new_ticket->description, description, DESC_BUFFER_SIZE - 1);
  new_ticket->description[DESC_BUFFER_SIZE - 1] = '\0';

  // Imposta la data corrente
  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  strftime(new_ticket->date, sizeof(new_ticket->date), "%d/%m/%Y", tm_info);

  // Imposta la priorità a bassa
  new_ticket->priority = LOW;
  // Imposta l'ID a 0 (sarà assegnato quando aggiunto alla lista)
  new_ticket->id = 0;

  // Imposta lo stato iniziale e l'agente
  new_ticket->status = OPEN;
  new_ticket->agentId = 0;
  new_ticket->userId = userId;

  return new_ticket;
}

Ticket *load_ticket_data(char *title, char *description, char *date,
                         unsigned int id, int priority, int status,
                         unsigned int agentId, unsigned int userId) {
  // Allocazione della memoria
  Ticket *ticket = malloc(sizeof(Ticket));
  if (ticket == NULL) {
    return NULL;
  }
  // Popolamento dei campi
  strncpy(ticket->title, title, sizeof(ticket->title) - 1);
  ticket->title[sizeof(ticket->title) - 1] = '\0';
  strncpy(ticket->description, description, sizeof(ticket->description) - 1);
  ticket->description[sizeof(ticket->description) - 1] = '\0';
  strncpy(ticket->date, date, sizeof(ticket->date) - 1);
  ticket->date[sizeof(ticket->date) - 1] = '\0';
  ticket->id = id;
  ticket->priority = priority;
  ticket->status = status;
  ticket->agentId = agentId;
  ticket->userId = userId;
  return ticket;
}

Ticket *clone_ticket(Ticket *ticket_source) {
  if (ticket_source == NULL) {
    return NULL;
  }
  Ticket *ticket_copy = malloc(sizeof(Ticket));
  if (ticket_copy == NULL) {
    perror("clone_ticket: malloc failed");
    return NULL;
  }
  memcpy(ticket_copy, ticket_source, sizeof(Ticket));
  return ticket_copy;
}

void set_ticket_agent(Ticket *ticket, unsigned int agentId) {
  ticket->agentId = agentId;
}

void set_ticket_priority(Ticket *ticket, Priority new_priority) {
  ticket->priority = new_priority;
}

void set_ticket_status(Ticket *ticket, Status new_status) {
  ticket->status = new_status;
}

char *ticket_to_string(Ticket *ticket) {
  // Alloca memoria per la stringa di output
  char *string = malloc(BUFFER_SIZE);
  if (string == NULL) {
    perror("print_ticket: malloc failed");
    return NULL;
  }

  // Conversione priorità
  const char *priority_str;
  switch (ticket->priority) {
  case LOW:
    priority_str = "LOW";
    break;
  case MEDIUM:
    priority_str = "MEDIUM";
    break;
  case HIGH:
    priority_str = "HIGH";
    break;
  default:
    priority_str = "LOW"; // default è il grado più basso
  }

  // Conversione stato
  const char *status_str;
  switch (ticket->status) {
  case OPEN:
    status_str = "OPEN";
    break;
  case IN_PROGRESS:
    status_str = "IN_PROGRESS";
    break;
  case CLOSED:
    status_str = "CLOSED";
    break;
  default:
    status_str = "OPEN"; // default stato aperto
  }
  snprintf(string, BUFFER_SIZE,
           "=====================\n"
           "ID: %d\n"
           "Titolo: %s\n"
           "Descrizione: %s\n"
           "Data: %s\n"
           "Priorità: %s\n"
           "Stato: %s\n"
           "Agente ID: %d\n"
           "Utente ID: %d\n"
           "=====================\n",
           ticket->id, ticket->title, ticket->description, ticket->date,
           priority_str, status_str, ticket->agentId, ticket->userId);
  return string;
}
