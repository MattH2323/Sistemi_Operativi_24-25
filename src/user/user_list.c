
#include "../../include/user/user_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/user/user.h"

#define INITIAL_CAPACITY 10
#define CAPACITY_GROWTH_FACTOR 2

// Dichiarazione dei prototipi
static unsigned int get_max_user_id(UserList *list);
static unsigned int generate_user_id(UserList *list);

UserList *user_list_init() {
  UserList *user_array = malloc(sizeof(UserList));
  if (!user_array) {
    perror("userlist malloc\n");
    return NULL;
  }
  // Alloca l'array di utenti
  user_array->users = malloc(INITIAL_CAPACITY * sizeof(User));
  if (!user_array->users) {
    perror("user malloc\n");
    return NULL;
  }
  // Inizializza il count e la capacity
  user_array->count = 0;
  user_array->capacity = INITIAL_CAPACITY;
  return user_array;
}

User *grow_user_list(UserList *user_list) {
  // Calcola la nuova capacità
  int new_capacity = (user_list->capacity == 0)
                         ? INITIAL_CAPACITY
                         : user_list->capacity * CAPACITY_GROWTH_FACTOR;
  User *new_users =
      (User *)realloc(user_list->users, new_capacity * sizeof(User));

  // Verifica se il realloc è andato a buon fine
  if (new_users == NULL) {
    perror("grow_user_list: realloc failed");
    return NULL;
  }
  // Aggiorna i riferimenti nella struttura
  user_list->users = new_users;
  user_list->capacity = new_capacity;
  return new_users;
}

User *authenticate_client(UserList *user_list, char *username, char *password) {
  User *user_found = get_user_by_username(user_list, username);
  if (user_found == NULL || strcmp(user_found->password, password) != 0) {
    return NULL;
  }
  // Creiamo una copia dell'utente trovato, invece di restituire il puntatore diretto
  User *authenticate_user = clone_user(user_found);
  if (authenticate_user == NULL) {
    return NULL;
  }
  return authenticate_user;
}

int add_user_to_list(UserList *list, User *user) {
  // Controlla se è necessario ridimensionare l'array
  if (list->count >= list->capacity) {
    if (grow_user_list(list) == NULL)
      return -1;
  }
  user->user_id = generate_user_id(list);
  list->users[list->count] = *user; // Copia l'utente nella lista
  list->count++;
  return 0;
}

User *get_user_by_id(UserList *ClientList, unsigned int user_id) {
  // Cerca l'utente per ID
  for (int i = 0; i < ClientList->count; i++) {
    if (ClientList->users[i].user_id == user_id) {
      return &ClientList->users[i];
    }
  }
  return NULL;
}

User *get_user_by_username(UserList *ClientList, char *username) {
  for (int i = 0; i < ClientList->count; i++) {
    if (strcmp(ClientList->users[i].username, username) == 0) {
      return &ClientList->users[i];
    }
  }
  return NULL;
}

User *get_user_by_email(UserList *ClientList, char *email) {
  for (int i = 0; i < ClientList->count; i++) {
    if (strcmp(ClientList->users[i].email, email) == 0) {
      return &ClientList->users[i];
    }
  }
  return NULL;
}

void clear_user_list(UserList *list) {
  if (list == NULL) {
    return;
  }
  free(list->users);
  list->users = NULL;
  list->count = 0;
  list->capacity = 0;
}

// ======================== Funzioni di supporto static =========================

// Trova il massimo user_id presente nella lista
static unsigned int get_max_user_id(UserList *list) {
  if (!list || list->count == 0)
    return 0;

  unsigned int max_id = 0;
  for (int i = 0; i < list->count; i++) {
    if (list->users[i].user_id > max_id) {
      max_id = list->users[i].user_id;
    }
  }
  return max_id;
}

// Genera un nuovo ID univoco
static unsigned int generate_user_id(UserList *list) {
  if (!list)
    return -1;
  return get_max_user_id(list) + 1;
}
