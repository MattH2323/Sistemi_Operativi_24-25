#ifndef NODE_H
#define NODE_H

#include "ticket.h"

/*
 * Rappresenta un nodo di una lista concatenata (linked list).
 * Ogni nodo contiene un riferimento a un Ticket e il collegamento al nodo
 * successivo.
 */
typedef struct Node {
  Ticket *data; // Puntatore ai dati del Ticket
  struct Node *next; // Puntatore al prossimo nodo della lista (o NULL se è l'ultimo)
} Node;

#endif