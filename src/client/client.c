#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../../include/client/client.h"
#include "../../include/client/input_validation.h"
#include "../../include/client/ticket_operations.h"
#include "../../include/client/user_operations.h"
#include "../../include/utils/communication.h"

// Dichiarazione dei prototipi
static OperationOutcome handle_input_signal(InputOutcome input_signal, const char *function_name);
static OperationOutcome get_and_send_menu_choice(int socket, int *choice);
static void clear_screen();

// Funzione main del client
int main(int argc, char *argv[]) {
  int clientSocket;
  struct sockaddr_in serverAddress;

  // apertura socket del client
  if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Error in socket()");
    exit(-1);
  }

  // preparazione dell'indirizzo del server
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(SERVER_PORT);
  serverAddress.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

  // connessione al server
  if ((connect(clientSocket, (struct sockaddr *)&serverAddress,
               sizeof(serverAddress))) == -1) {
    perror("Error in connect()");
    exit(-1);
  }

  int connected = 1; // Variabile per gestire la connessione
  printf("Client connesso a %s.\n", SERVER_ADDRESS);

  // Gestisce l'interazione con i menu del server
  request_session(clientSocket);

  close(clientSocket);
  printf("Connessione chiusa.\n");
  return 0;
}

OperationOutcome request_session(int socket) {
  char user_type[USER_TYPE_LENGTH];
  OperationOutcome outcome;

  clear_screen();

  printf("[ISTRUZIONI]\n"
                 "Per selezionare un opzione del menu, inserire il numero corrispondente.\n"
                 "Per annullare l'inserimento in corso, digitare ESC + INVIO.\n"
                 "Premere INVIO per continuare...\n");
  getchar();

  while (true) {
    memset(user_type, 0, USER_TYPE_LENGTH);
    // Autenticazione dell'utente
    outcome = authentication_menu(socket, user_type);
    if (outcome == POSITIVE) {
      // caso di registrazione con successo
      if (strcmp(user_type, CLIENT_STRING) == 0) {
        outcome = client_main_menu(socket);
      } else if (strcmp(user_type, AGENT_STRING) == 0) {
        outcome = agent_main_menu(socket);
      }
    }
    if (outcome != POSITIVE) {
      break;
    }
  }
  return outcome;
}

OperationOutcome authentication_menu(int socket, char *user_type) {
  OperationOutcome outcome;
  int choice;

  const char access_menu[] = "=== Menu di Accesso ===\n"
                             "1. Login\n"
                             "2. Registrazione\n"
                             "3. Esci\n"
                             "Scegli un'opzione (1-3):\n";

  while (true) {

    clear_screen();

    printf("%s", access_menu);

    // Invio della scelta dell'utente
    outcome = get_and_send_menu_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    // Gestione della scelta dell'utente
    switch (choice) {
    case 1:
      // Login
      outcome = login(socket, user_type);
      break;
    case 2:
      // Registrazione
      outcome = registration(socket, user_type);
      break;
    case 3:
      // Uscita esplicita
      outcome = NEGATIVE;
      return outcome;
    default:
      printf("Scelta non valida. Riprova(1-3)\n");
      break;
    }
    if (outcome == POSITIVE) {
      return POSITIVE;
    } else if (outcome == ERROR) {
      return ERROR;
    } else {
      // Pausa prima di tornare al menu
      printf("Premi INVIO per continuare...\n");
      getchar();
    }
  }
  return outcome;
}

OperationOutcome client_main_menu(int socket) {
  OperationOutcome outcome;
  int choice;

  const char client_menu[] = "=== Menu Cliente ===\n"
                             "1. Crea Ticket\n"
                             "2. Visualizza Ticket\n"
                             "3. Logout\n"
                             "Scegli un'opzione (1-3): \n";

  while (true) {

    clear_screen();

    printf("%s", client_menu);

    // Invio della scelta dell'utente
    outcome = get_and_send_menu_choice(socket, &choice);

    // Se l'input fallisce (EOF) o l'utente annulla (ESC), usciamo
    if (outcome != POSITIVE) {
      return outcome;
    }

    // Gestione della scelta dell'utente
    switch (choice) {
    case 1:
      // Crea Ticket
      outcome = ticket_creation(socket);
      break;
    case 2:
      // Consulta Ticket (Sottomenu di ricerca)
      outcome = ticket_consultation_menu(socket);
      break;
    case 3:
      // Logout esplicito
      printf("Logout effettuato.\n");
      return POSITIVE;
    default:
      printf("Scelta non valida. Riprova (1-3).\n");
      break;
    }

    if (outcome == ERROR) {
      return ERROR;
    } else {
      // Pausa prima di tornare al menu
      printf("Premi INVIO per continuare...\n");
      getchar();
    }
  }
  return outcome;
}

OperationOutcome agent_main_menu(int socket) {
  OperationOutcome outcome;
  int choice;

  const char agent_menu[] = "=== Menu Agente ===\n"
                            "1. Consulta ticket\n"
                            "2. Gestisci ticket\n"
                            "3. Visualiza ticket liberi (Non assegnati)\n"
                            "4. Prendi in carico\n"
                            "5. Logout\n"
                            "Scegli un'opzione (1-5): \n";

  while (true) {

    clear_screen();

    printf("%s", agent_menu);

    // Invio della scelta
    outcome = get_and_send_menu_choice(socket, &choice);

    // Se l'input fallisce (EOF) o l'utente annulla (ESC), usciamo
    if (outcome != POSITIVE) {
      return outcome;
    }

    // Gestione della scelta dell'utente
    switch (choice) {
    case 1:
      // Visualizza Ticket
      outcome = ticket_consultation_menu(socket);
      break;
    case 2:
      // Gestisci Ticket (Modifica stato/priorità)
      outcome = ticket_management_menu(socket);
      break;
    case 3:
      // Visualizza ticket liberi
      outcome = ticket_list_sorting(socket);
      break;
    case 4:
      // Assegna agente (Prendi in carico il ticket)
      outcome = ticket_assignment(socket);
      break;
    case 5:
      // Logout esplicito
      printf("Logout effettuato.\n");
      return POSITIVE;
    default:
      printf("Scelta non valida. Riprova (1-5).\n");
      break;
    }

    if (outcome == ERROR) {
      return ERROR;
    } else {
      // Pausa prima di tornare al menu
      printf("Premi INVIO per continuare...\n");
      getchar();
    }
  }
  return outcome;
}

OperationOutcome ticket_consultation_menu(int socket) {
  OperationOutcome outcome;
  int choice;

  const char consult_menu[] = "=== Consultazione Ticket ===\n"
                              "1. Cerca per [ID] (propri ticket)\n"
                              "2. Cerca per [Titolo] - case insensitive (propri ticket)\n"
                              "3. Cerca per [Data] (propri ticket)\n"
                              "4. Stampa ordinati per priorità (Decrescente)\n"
                              "5. Stampa tutti i miei ticket\n"
                              "6. Torna al menu principale\n"
                              "Scegli un'opzione (1-6): \n";

  while (true) {

    clear_screen();

    printf("%s", consult_menu);

    // Invio della scelta
    outcome = get_and_send_menu_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    switch (choice) {
    case 1:
      // Ricerca Specifica per ID
      outcome = ticket_searching_by_id(socket);
      break;
    case 2:
      // Ricerca per sottstringa del Titolo
      outcome = ticket_list_searching_by_field(socket, SUBSTRING);
      break;
    case 3:
      // Ricerca per Data
      outcome = ticket_list_searching_by_field(socket, DATE);
      break;
    case 4:
      // Ordinamento per PRIORITÀ
      outcome = ticket_list_sorting(socket);
      break;
    case 5:
      // Stampa tutti i ticket (Ordine default)
      outcome = ticket_list_sorting(socket);
      break;
    case 6:
      // Torna al menu principale (Logout dal sottomenu)
      return POSITIVE;
    default:
      printf("Scelta non valida. Riprova (1-6).\n");
      break;
    }
    if (outcome == ERROR) {
      return ERROR;
    } else {
      // Pausa prima di tornare al menu
      printf("Premi INVIO per continuare...\n");
      getchar();
    }
  }
  return outcome;
}

OperationOutcome ticket_management_menu(int socket) {
  OperationOutcome outcome;
  int choice;

  // Menu per la gestione dei ticket (modifica priorità/stato)
  const char ticket_management[] = "=== Gestione Ticket ===\n"
                                   "1. Modifica priorità di un ticket\n"
                                   "2. Modifica stato di un ticket\n"
                                   "3. Torna al menu principale\n"
                                   "Scegli un'opzione (1-3): \n";

  while (true) {

    clear_screen();

    printf("%s", ticket_management);

    // Invio della scelta
    outcome = get_and_send_menu_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    switch (choice) {
    case 1:
      // Modifica Priorità
      outcome = ticket_modification(socket, PRIORITY);
      break;
    case 2:
      // Modifica Stato
      outcome = ticket_modification(socket, STATUS);
      break;
    case 3:
      // Torna al menu principale (Logout dal sottomenu)
      return POSITIVE;
    default:
      printf("Scelta non valida. Riprova (1-3).\n");
      break;
    }
    // Se c'è un errore di connessione durante le operazioni, usciamo dal ciclo
    if (outcome == ERROR) {
      return ERROR;
    } else {
      // Pausa prima di tornare al menu
      printf("Premi INVIO per continuare...\n");
      getchar();
    }
  }

  return outcome;
}

// ======================== Funzioni di supporto static =========================

// Acquisisce l'input del menu e lo invia al server 
static OperationOutcome get_and_send_menu_choice(int socket, int *choice) {
  // Acquisisce l'input
  InputOutcome input_outcome = get_int_stdin(choice);

  if (input_outcome == INPUT_SUCCESS) {
    // Invia la scelta al server
    CommunicationStatus comm_status = send_int(socket, *choice);
    if (comm_status != COMM_SUCCESS) {
      // Gestione errore di comunicazione
      return handle_communication_signal(comm_status, __func__);
    }
    return POSITIVE;
  } else {
    // Gestione errore di input
    return handle_input_signal(input_outcome, __func__);
  }
  return ERROR;
}

// Conversione del segnale di input in esito operazione
static OperationOutcome handle_input_signal(InputOutcome input_outcome,
                                            const char *function_name) {
  switch (input_outcome) {
  case INPUT_SUCCESS:
    // Input valido -> Operazione riuscita
    return POSITIVE;
  case INPUT_CANCEL:
    // Input annullato dall'utente -> Operazione con esito negativo
    return NEGATIVE;
  case INPUT_EOF:
    // Input EOF -> Interruzione della comunicazione
    fprintf(stderr, "[Input Terminato] In: %s\n", function_name);
    return EOF;
  case INPUT_ERROR:
    // Errore di input -> Errore di sistema
    return ERROR;
  default:
    fprintf(stderr, "[Errore Input] In: %s\n", function_name);
    return ERROR;
  }
}

// Pulizia schermo del terminale
static void clear_screen() { system("clear"); }
