#ifndef USER_OPERATIONS_HANDLER_H
#define USER_OPERATIONS_HANDLER_H

#include "../user/user.h"
#include "../utils/communication.h"

// Gestione del login di un utente (Se riuscito, restituisce l'utente tramite il puntatore passato)
OperationOutcome handle_login(int socket, User **user);

// Gestione della registrazione di un nuovo utente (Se riuscito, restituisce il nuovo utente tramite il puntatore passato)
OperationOutcome handle_registration(int socket, User **new_user);

#endif