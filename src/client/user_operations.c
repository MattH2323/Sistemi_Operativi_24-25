#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../../include/client/input_validation.h"
#include "../../include/client/user_operations.h"
#include "../../include/utils/communication.h"
#include "../../include/utils/config.h"

OperationOutcome login(int socket, char *user_type) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  char username[USERNAME_BUFFER_SIZE];
  char password[PASSWORD_BUFFER_SIZE];

  // Richiesta Username
  printf("Inserisci username:\n");
  input_outcome = get_username_stdin(username);

  if (input_outcome == INPUT_SUCCESS) {
    // Richiesta Password
    printf("Inserisci password:\n");
    input_outcome = get_password_stdin(password);
  }

  // Handshake di disponibilità dati
  availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Invio delle credenziali
  comm_status = send_string(socket, username);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  comm_status = send_string(socket, password);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Ricevi l'esito dell'autenticazione
  comm_status = receive_outcome(socket, &op_outcome);

  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Feedback all'utente
  if (op_outcome == POSITIVE) {
    printf("Login avvenuto con successo!\n");
    comm_status = receive_string(socket, user_type, USER_TYPE_LENGTH);
    if (comm_status != COMM_SUCCESS) {
      return handle_communication_signal(comm_status, __func__);
    }
  } else if (op_outcome == NEGATIVE) {
    printf("Login fallito. Riprova.\n");
  } else {
    fprintf(stderr, "Errore di sistema lato server durante il login.\n");
  }

  return op_outcome;
}

OperationOutcome registration(int socket, char *user_type) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  char username[USERNAME_BUFFER_SIZE];
  char password[PASSWORD_BUFFER_SIZE];
  char email[EMAIL_BUFFER_SIZE];

  // Richiesta Username
  printf("Inserisci username (MAX %d caratteri):\n", USERNAME_MAX_LEN);
  input_outcome = get_username_stdin(username);

  if (input_outcome == INPUT_SUCCESS) {
    // Richiesta Password
    printf("Inserisci password (MAX %d caratteri):\n", PASSWORD_MAX_LEN);
    input_outcome = get_password_stdin(password);
  }

  if (input_outcome == INPUT_SUCCESS) {
    // Richiesta Email
    printf("Inserisci email (MAX %d caratteri):\n", EMAIL_MAX_LEN);
    input_outcome = get_mail_stdin(email);
  }

  // Handshake di disponibilità dati
  availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Invio dei dati al server
  comm_status = send_string(socket, username);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  comm_status = send_string(socket, password);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  comm_status = send_string(socket, email);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Attendi la validazione della registrazione dal server
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Feedback all'utente
  if (op_outcome == POSITIVE) {
    printf("Registrazione avvenuta con successo!\n");
    strcpy(user_type, CLIENT_STRING);
  } else if (op_outcome == NEGATIVE) {
    printf("Registrazione fallita: Username o email già esistenti.\n");
  } else {
    fprintf(stderr,
            "Errore di sistema lato server durante la registrazione.\n");
  }
  return op_outcome;
}
