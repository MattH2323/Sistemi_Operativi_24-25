#ifndef TICKET_LIST_H
#define TICKET_LIST_H

#include "stdlib.h"

#include "node.h"

/*
 * Rappresenta una lista concatenata di Ticket.
 * Questa struttura funge da contenitore principale per gestire una collezione di ticket,
 * mantenendo il puntatore al primo nodo e il conteggio totale degli elementi
 * per facilitare l'iterazione e il monitoraggio della dimensione della lista.
 */
typedef struct TicketList {
  Node *head;         // Puntatore al primo nodo della lista
  unsigned int count; // Numero di elementi nella lista
} TicketList;

// Inizializzazione della ticket_list
TicketList *ticket_list_init();

// Aggiunta di un ticket alla lista
int add_ticket(TicketList *tickets, Ticket *current_ticket);

// Ricerca di un ticket per ID
Ticket *get_ticket_by_id(TicketList *tickets, unsigned int ticket_id);

// Ricerca di un ticket per posizione nella lista di n elementi (index = 0..n-1)
Ticket *get_by_index(TicketList *list, int index);

// Ricerca di ticket per data
TicketList *filter_by_date(TicketList *tickets, char *date);

// Ricerca di ticket per sottostringa nel titolo (case insensitive)
TicketList *filter_by_title_substring(TicketList *tickets, char *substring);

// Ricerca di ticket per ID utente
TicketList *filter_by_user_id(TicketList *tickets, unsigned int user_id);

// Ricerca di ticket per ID agente
TicketList *filter_by_agent_id(TicketList *tickets, unsigned int agent_id);

// Ordinamento per priorità (decrescente)
void sort_tickets_by_priority(TicketList *ticket_list);

// Liberazione della memoria allocata per la lista dei ticket
void clear_ticket_list(TicketList *tickets);

// Liberazione della memoria allocata per la lista dei nodi di ticket (senza deallocare il campo data)
void clear_ticket_list_without_data(TicketList *tickets);

#endif