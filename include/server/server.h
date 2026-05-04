#ifndef SERVER_H
#define SERVER_H

#include "../user/user.h"
#include "../utils/communication.h"
#include "ticket_operations_handler.h"
#include "user_operations_handler.h"

// Gestione della sessione
void handle_session(int socket);

// Gestione dell'autenticazione degli utenti 
OperationOutcome handle_authentication_menu(int socket, User **user);

// Gestione menù richiesta cliente
OperationOutcome handle_client_menu(int socket, User *current_user);

// Gestione menù richiesta agente
OperationOutcome handle_agent_menu(int socket, User *current_user);

// Gestione menù consultazione ticket
OperationOutcome handle_ticket_consultation_menu(int socket,
                                                 unsigned int current_user_id,
                                                 UserType user_type);

// Gestione menù modifica ticket
OperationOutcome handle_ticket_management_menu(int socket,
                                               unsigned int current_user_id);

#endif