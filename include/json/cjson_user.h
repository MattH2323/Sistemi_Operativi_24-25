#ifndef CJSON_USER_H
#define CJSON_USER_H

#include "../user/user_list.h"

#define USERS_FILE "data/users.json"

// Caricamento della UserList dal file JSON (Se aperto in modalità scrittura, necessita la chiusura manuale con save_user_list)
UserList *load_user_list(const char *mode);

// Salva la UserList nel file JSON. Restituisce 0 o -1 in caso di errore.
int save_user_list(UserList *user_list);

#endif // CJSON_USER_H