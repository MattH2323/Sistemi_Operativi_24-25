#ifndef TICKET_OPERATIONS_H
#define TICKET_OPERATIONS_H

#include "../utils/communication.h"
#include "input_validation.h"

// Richiesta creazione ticket
OperationOutcome ticket_creation(int socket);

// Richiesta ricerca ticket per ID
OperationOutcome ticket_searching_by_id(int socket);

// Richiesta ricerca ticket per campo specifico (titolo/data)
OperationOutcome ticket_list_searching_by_field(int socket, const char* mode) ;

// Richiesta lista ticket ordinata
OperationOutcome ticket_list_sorting(int socket);

// Richiesta modifica ticket
OperationOutcome ticket_modification(int socket, const char * mode);

// Richiesta assegnazione ticket
OperationOutcome ticket_assignment(int socket);

#endif // TICKET_OPERATIONS_H