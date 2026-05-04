#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/user/user.h"

User *create_new_user(const char *username, const char *password,
                      const char *email, UserType type) {
  User *new_user = malloc(sizeof(User));
  if (new_user == NULL) {
    perror("create_user: malloc failed");
    return NULL;
  }
  // Inizializzazione dei campi
  strncpy(new_user->username, username, USERNAME_BUFFER_SIZE - 1);
  new_user->username[USERNAME_BUFFER_SIZE - 1] = '\0';

  strncpy(new_user->password, password, PASSWORD_BUFFER_SIZE - 1);
  new_user->password[PASSWORD_BUFFER_SIZE - 1] = '\0';

  strncpy(new_user->email, email, EMAIL_BUFFER_SIZE - 1);
  new_user->email[EMAIL_BUFFER_SIZE - 1] = '\0';

  new_user->type = type;

  return new_user;
}

User *clone_user(User *user_found) {
  User *new_user = malloc(sizeof(User));
  if (new_user == NULL) {
    perror("create_user: malloc failed");
    return NULL;
  }
  strcpy(new_user->username, user_found->username);
  strcpy(new_user->password, user_found->password);
  strcpy(new_user->email, user_found->email);
  new_user->type = user_found->type;
  new_user->user_id = user_found->user_id;

  return new_user;
}
