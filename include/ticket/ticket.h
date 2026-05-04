#ifndef TICKET_H
#define TICKET_H

#include "../utils/config.h"

// Definizione tipi enumerati per priorità: HIGH, MEDIUM, LOW
typedef enum { LOW = 1, MEDIUM, HIGH } Priority;

// Definizione tipi enumerati per stato: OPEN, IN_PROGRESS, CLOSED
typedef enum { OPEN = 1, IN_PROGRESS, CLOSED } Status;

/*
 * Rappresenta un ticket di supporto o segnalazione all'interno del sistema.
 * Ogni ticket raggruppa le informazioni descrittive del problema, i metadati temporali,
 * lo stato di gestione e i riferimenti all'utente e all'agente assegnato.
 */
typedef struct Ticket {
  char title[TITLE_BUFFER_SIZE];
  char description[DESC_BUFFER_SIZE];
  char date[DATE_BUFFER_SIZE];
  unsigned int id;
  Priority priority;            // Priorità del ticket: LOW, MEDIUM, HIGH
  Status status;                // Stato del ticket: OPEN, IN_PROGRESS, CLOSED
  unsigned int agentId;
  unsigned int userId;
} Ticket;

// Creazione ticket
Ticket* create_ticket(char *title, char *description, unsigned int userId);

// Caricamento dati ticket
Ticket* load_ticket_data(char *title, char *description,
                         char *date, unsigned int id, int priority, int status,
                         unsigned int agentId, unsigned int userId) ;

// Clonazione ticket
Ticket* clone_ticket (Ticket * ticket_source);

// Modifica agente
void set_ticket_agent(Ticket *ticket, unsigned int agentId);

// Modifica priorità
void set_ticket_priority(Ticket *ticket, Priority new_priority);

// Modifica stato
void set_ticket_status(Ticket *ticket, Status new_status);

// Genera stringa di rappresentazione del ticket
char *ticket_to_string(Ticket *ticket);

#endif