#ifndef CJSON_TICKET_H
#define CJSON_TICKET_H

#include "../ticket/ticket_list.h"

#define TICKETS_FILE "data/tickets.json"

// Caricamento della ticket_list da file JSON(con lock esclusivo se in scrittura)
TicketList *load_ticket_list(const char *mode);

// Salvataggio della ticket_list su file JSON (assume lock esclusivo aperto)
int save_ticket_list(TicketList *tickets);

#endif // CJSON_TICKET_H