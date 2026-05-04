#ifndef USER_LIST_H
#define USER_LIST_H

#include "stdlib.h"

#include "user.h"

/*
 * Rappresenta una collezione di utenti gestita tramite un array dinamico
 * ridimensionabile. La struttura mantiene il puntatore al buffer di memoria, il
 * conteggio degli utenti attuali e la capacità totale allocata per gestire
 * l'espansione automatica.
 */
typedef struct UserList {
  User *users;           // L'array dinamico
  unsigned int count;    // contatore di utenti
  unsigned int capacity; // capacità dell'array
} UserList;

// Valori di default per la user_list (Array dinamico)
#define INITIAL_CAPACITY 10
#define CAPACITY_GROWTH_FACTOR 2

// Inizializzazione della user_list
UserList *user_list_init();

// Rialloca la user_list per aumentare la capacità
User *grow_user_list(UserList *user_list);

// Autenticazione utente (Ricerca username e password)
User *authenticate_client(UserList *user_list, char *username, char *password);

// Aggiunta di un utente alla lista con generazione di un ID unico
int add_user_to_list(UserList *user_list, User *new_user);

// Ricerca di un utente per ID
User *get_user_by_id(UserList *ClientList, unsigned int user_id);

// Ricerca di un utente per username
User *get_user_by_username(UserList *ClientList, char *username);

// Ricerca di un utente per email
User *get_user_by_email(UserList *ClientList, char *email);

// Pulizia della memoria allocata per la lista utenti
void clear_user_list(UserList *user_list);

#endif // USER_LIST_H