#ifndef CLIENT_H
#define CLIENT_H

#include "../utils/communication.h"


// Gestione della sessione lato client
OperationOutcome request_session(int socket);

// Menu di autenticazione al server
OperationOutcome authentication_menu(int socket, char* user_type);

// Menù principale del client
OperationOutcome client_main_menu(int socket);

// Menù per la richiesta agente
OperationOutcome agent_main_menu(int socket);

// Menù per la richiesta consultazione ticket
OperationOutcome ticket_consultation_menu(int socket);

// Menù per la richiesta di gestione ticket
OperationOutcome ticket_management_menu(int socket);



#endif // CLIENT_H