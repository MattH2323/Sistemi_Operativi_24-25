#ifndef TICKET_OPERATIONS_HANDLER_H
#define TICKET_OPERATIONS_HANDLER_H

#include "../ticket/ticket.h"
#include "../user/user.h"
#include "../utils/communication.h"

// Gestisce la creazione di un nuovo ticket
OperationOutcome handle_ticket_creation(int socket,
                                        unsigned int current_user_id);

// Ricerca di un ticket per ID e invio del risultato
OperationOutcome handle_ticket_searching_by_id(int socket,
                                               unsigned int current_user_id,
                                               UserType user_type);

// Ricerca di ticket per campo specifico (titolo/data) e invio del risultato
OperationOutcome handle_ticket_list_searching_by_field(int socket, 
                                                       unsigned int current_user_id,
                                                       UserType user_type, 
                                                       const char *mode);

// Ricerca di ticket non assegnati e invio del risultato
OperationOutcome handle_ticket_list_searching_by_unassigned(int socket);

// Stampa e/o ordina la lista dei ticket
OperationOutcome handle_ticket_list_sorting(int socket,
                                            unsigned int current_user_id,
                                            UserType user_type,
                                            const char *sort_type);

// Gestione agenti ticket
OperationOutcome handle_ticket_modification(int socket,
                                            unsigned int current_user_id,
                                            const char *modify_type);

// Assegna un ticket all'agente corrente
OperationOutcome handle_ticket_assignment(int socket,
                                          unsigned int current_user_id);

// Invia la lista dei ticket assegnati ad un agente
OperationOutcome send_assigned_ticket_list(int socket,
                                           unsigned int current_user_id);

#endif