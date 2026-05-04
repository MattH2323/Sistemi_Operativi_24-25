#ifndef USER_H
#define USER_H

#include "../utils/config.h"

/*
 * Definisce i ruoli possibili per un utente all'interno del sistema.
 * Utilizzato per distinguere tra clienti (richiedenti supporto) e agenti
 * (operatori del supporto).
 */
typedef enum {
  CLIENT,
  AGENT,
} UserType;

/*
 * Rappresenta un account utente registrato nel database del sistema.
 * Raggruppa le credenziali di accesso, i dati identificativi personali e il
 * ruolo (tipo) necessario per gestire l'autenticazione e le autorizzazioni.
 */
typedef struct User {
  char username[USERNAME_BUFFER_SIZE];
  char password[PASSWORD_BUFFER_SIZE];
  unsigned int user_id;
  char email[EMAIL_BUFFER_SIZE];
  UserType type; // Tipo di utente: CLIENT o AGENT
} User;

// Creazione nuovo utente (senza assegnazione ID)
User *create_new_user(const char *username, const char *password,
                      const char *email, UserType type);

// Copia un utente esistente
User *clone_user(User *user_found);

#endif
