#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/ticket/ticket_list.h"

// Dichiarazione dei prototipi
static Node *create_node();
static unsigned int generate_ticket_id(TicketList *tickets);
static unsigned int get_max_ticket_id(TicketList *ticket_list);

TicketList *ticket_list_init() {
  TicketList *new_list = malloc(sizeof(TicketList));
  if (!new_list) {
    perror("create_list: malloc failed");
    return NULL;
  }
  // Inizializza i campi della lista con valori di default
  new_list->head = NULL;
  new_list->count = 0;
  return new_list;
}

int add_ticket(TicketList *tickets, Ticket *current_ticket) {
  Node *new_node = create_node();
  if (new_node == NULL) {
    return -1;
  }
  // Assegna un ID univoco se non è già presente 
  if (current_ticket->id == 0) {
    current_ticket->id = generate_ticket_id(tickets);
  }
  // Aggiunge il nuovo nodo all'inizio della lista
  new_node->data = current_ticket;
  new_node->next = tickets->head;
  tickets->head = new_node;
  tickets->count++;
  return 0;
}

Ticket *get_by_index(TicketList *list, int index) {
  if (index < 0 || index >= list->count)
    return NULL;

  Node *current = list->head;
  int i = 0;
  while (current) {
    if (i == index)
      return current->data;
    current = current->next;
    i++;
  }
  return NULL;
}

Ticket *get_ticket_by_id(TicketList *tickets, unsigned int ticket_id) {
  Node *node_found = tickets->head;
  Ticket *ticket_found = NULL;

  while (node_found != NULL) {
    ticket_found = node_found->data;
    // Confronta l'ID del ticket corrente con quello cercato
    if (ticket_found->id == ticket_id) {
      return ticket_found;
    }
    node_found = node_found->next;
  }
  return NULL;
}

TicketList *filter_by_date(TicketList *tickets, char *date) {
  Node *current_node = tickets->head;
  TicketList *filtered_ticket_list = ticket_list_init();

  if (filtered_ticket_list == NULL) {
    return NULL;
  }
  // Scorre tutta la lista dei ticket
  while (current_node) {
    Ticket *current_ticket = current_node->data;
    if (strcmp(current_ticket->date, date) == 0) {
      // Aggiunge il ticket alla lista filtrata
      if (add_ticket(filtered_ticket_list, current_ticket) == -1) {
        clear_ticket_list(filtered_ticket_list);
        return NULL;
      }
    }
    current_node = current_node->next;
  }

  return filtered_ticket_list;
}

TicketList *filter_by_user_id(TicketList *tickets, unsigned int user_id) {
  TicketList *filtered_ticket_list = ticket_list_init();
  if (filtered_ticket_list == NULL)
    return NULL;
  Node *current_node = tickets->head;

  // Scorre tutta la lista dei ticket
  while (current_node) {
    Ticket *current_ticket = current_node->data;
    if (current_ticket->userId == user_id) {
      // Aggiunge il ticket alla lista filtrata
      if (add_ticket(filtered_ticket_list, current_ticket) == -1) {
        clear_ticket_list(filtered_ticket_list);
        return NULL;
      }
    }
    current_node = current_node->next;
  }

  return filtered_ticket_list;
}

TicketList *filter_by_agent_id(TicketList *tickets, unsigned int agent_id) {
  TicketList *filtered_ticket_list = ticket_list_init();
  if (filtered_ticket_list == NULL)
    return NULL;
  Node *current_node = tickets->head;

  // Scorre tutta la lista dei ticket
  while (current_node) {
    Ticket *current_ticket = current_node->data;
    if (current_ticket->agentId == agent_id) {
      // Aggiunge il ticket alla lista filtrata
      if (add_ticket(filtered_ticket_list, current_ticket) == -1) {
        clear_ticket_list(filtered_ticket_list);
        return NULL;
      }
    }
    current_node = current_node->next;
  }

  return filtered_ticket_list;
}

TicketList *filter_by_title_substring(TicketList *tickets, char *substring) {
  TicketList *filtered_ticket_list = ticket_list_init();
  if (filtered_ticket_list == NULL)
    return NULL;
  Node *current_node = tickets->head;

  while (current_node) {
    Ticket *current_ticket = current_node->data;
    if (strcasestr(current_ticket->title, substring) != NULL) {
      if (add_ticket(filtered_ticket_list, current_ticket) == -1) {
        clear_ticket_list(filtered_ticket_list);
        return NULL;
      }
    }
    current_node = current_node->next;
  }
  return filtered_ticket_list;
}

void sort_tickets_by_priority(TicketList *ticket_list) {
  if (ticket_list == NULL || ticket_list->head == NULL)
    return;
  int swapped;
  Node *last_pointer = NULL;
  // Bubble sort
  do {
    swapped = 0;
    Node *prev = NULL;
    Node *curr = ticket_list->head;
    while (curr->next != last_pointer) {
      if (curr->data->priority > curr->next->data->priority) {
        Node *tmp = curr->next;
        // Scambio dei nodi adiacenti
        curr->next = tmp->next;
        tmp->next = curr;
        if (prev == NULL)
          ticket_list->head = tmp; // aggiorna head se serviva
        else
          prev->next = tmp;
        // dopo lo swap, 'next' viene prima di 'curr'
        prev = tmp;
        swapped = 1;
      } else {
        prev = curr;
        curr = curr->next;
      }
    }
    last_pointer = curr;
  } while (swapped);
}

void clear_ticket_list(TicketList *tickets) {
  if (tickets == NULL)
    return;

  // Liberazione spazio di memotia dei nodi e dei ticket
  Node *current_node = tickets->head;
  while (current_node) {
    Node *tmp = current_node;
    current_node = current_node->next;
    free(tmp->data);
    free(tmp);
  }
  // Liberazione spazio di memoria della lista ticket
  free(tickets);
}

void clear_ticket_list_without_data(TicketList *tickets) {
  if (tickets == NULL)
    return;

  // Liberazione spazio di memotia dei nodi e dei ticket
  Node *current_node = tickets->head;
  while (current_node) {
    Node *tmp = current_node;
    current_node = current_node->next;
    free(tmp);
  }
  // Liberazione spazio di memoria della lista ticket
  free(tickets);
}

// Creazione di un nuovo nodo vuoto
static Node *create_node() {
  Node *new_node = malloc(sizeof(Node));
  if (new_node == NULL) {
    perror("create_empty_node: malloc failed");
    return NULL;
  }
  // Inizializza i campi del nodo con valori di default
  new_node->data = NULL;
  new_node->next = NULL;
  return new_node;
}

// ======================== Funzioni di supporto static =========================

// Trova il massimo ID attualmente presente nella lista
static unsigned int get_max_ticket_id(TicketList *ticket_list) {
  // Se la lista non esiste o è vuota, il massimo ID è considerato 0
  if (ticket_list == NULL || ticket_list->head == NULL) {
    return 0; 
  }

  unsigned int max_id = 0;
  Node *current = ticket_list->head;

  while (current != NULL) {
    if (current->data != NULL) {
        if (current->data->id > max_id) {
            max_id = current->data->id;
        }
    }
    current = current->next;
  }
  return max_id;
}

// Genera un nuovo ID unico per un ticket
static unsigned int generate_ticket_id(TicketList *tickets) {
  // Calcola il max attuale e aggiunge 1
  return get_max_ticket_id(tickets) + 1;
}