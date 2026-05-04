#ifndef USER_OPERATIONS_H
#define USER_OPERATIONS_H

#include "../utils/communication.h"

// Richiesta di login al server
OperationOutcome login(int socket, char* user_type);

// Richiesta di registrazione al server
OperationOutcome registration(int socket, char* user_type);

#endif // USER_OPERATIONS_H