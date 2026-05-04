#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../include/server/server.h"
#include "../../include/server/ticket_operations_handler.h"
#include "../../include/server/user_operations_handler.h"
#include "../../include/utils/communication.h"


// Dichiarazione dei prototipi
static void handle_sigchild(int sig);
static OperationOutcome receive_client_choice(int socket, int *choice);

int main(int argc, char *argv[]) {
  int server_socket;
  struct sockaddr_in serverAddress, clientAddress;

  // Creazione socket server 
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("main: socket failed");
    exit(-1);
  }

  // Preparazione dell'indirizzo locale del server 
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(SERVER_PORT);
  serverAddress.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

  // Bind del socket 
  if (bind(server_socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
    perror("main: bind failed");
    exit(-1);
  }

  // Listen del socket (max 5 connessioni)
  if (listen(server_socket, 5) == -1) {
    perror("main: listen failed");
    exit(-1);
  }
  printf("Server ready (CTRL-C per terminare)\n");

  socklen_t clientAddressLen = sizeof(clientAddress);
  int connect_socket;
  char *clientIP;

  // Ignora il segnale SIGTSTP (CTRL-Z)
  signal(SIGTSTP, SIG_IGN);

  // Gestione del segnale SIGCHLD per evitare processi zombie
  signal(SIGCHLD, handle_sigchild);

  while (true) {
    // Accetta una connessione in ingresso
    printf("Waiting for a client connection...\n");

    connect_socket = accept(server_socket, (struct sockaddr *)&clientAddress,&clientAddressLen);
    if (connect_socket == -1) {
      perror("main: accept failed");
      continue; // Continua il ciclo in caso di errore
    }

    int pid = fork(); // Creazione di un processo figlio

    if (pid == 0) { // Processo figlio
      // memorizzo l'indirizzo del client in formato dots 
      clientIP = inet_ntoa(clientAddress.sin_addr);

      printf("\nClient @ %s connects on socket %d\n", clientIP, connect_socket);

      handle_session(connect_socket); // Gestisci sessione client
      close(connect_socket);          // Processo figlio chiude il socket di
                                      // connessione
    }
  }
  close(server_socket); // Processo padre chiude il socket di ascolto
  return 0;
}

void handle_session(int socket) {
  OperationOutcome outcome;
  User *user = NULL;

  while (true) {
    // Autenticazione dell'utente
    outcome = handle_authentication_menu(socket, &user);

    if (outcome != POSITIVE) {
      // Liberiamo la memoria dell'utente in caso di errore o logout
      if (user) {
        free(user);
        user = NULL;
      }
      return;
    }

    // Autenticazione riuscita
    if (user->type == CLIENT) {
      outcome = handle_client_menu(socket, user);
    } else if (user->type == AGENT) {
      outcome = handle_agent_menu(socket, user);
    }

    // Liberiamo la memoria dell'utente
    if (user) {
      free(user);
      user = NULL;
    }
    // Se si è verificato un errore di comunicazione, usciamo
    if (outcome == ERROR) {
      return;
    }
  }
}

OperationOutcome handle_authentication_menu(int socket, User **user) {
  OperationOutcome outcome;
  int choice;

  while (true) {

    // Attesa della scelta
    outcome = receive_client_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    // Gestione della scelta dell'utente
    switch (choice) {
    case 1:
      outcome = handle_login(socket, user);
      break;
    case 2:
      outcome = handle_registration(socket, user);
      break;
    case 3:
      return NEGATIVE; // Non autenticato, esci
    default:
      // Scelta non valida: il ciclo ricomincia
      continue;
    }

    // Se l'autenticazione ha avuto successo, usciamo ritornando POSITIVE
    if (*user != NULL && outcome == POSITIVE) {
      return POSITIVE;
    }
    // Se si è verificato un errore fatale, usciamo ritornando ERROR
    if (outcome == ERROR) {
      return ERROR;
    }
    // Se outcome è NEGATIVE (es. login fallito), il ciclo ricomincia
  }
}

OperationOutcome handle_client_menu(int socket, User *current_user) {
  OperationOutcome outcome;
  int choice;

  while (true) {
    // Attendi la scelta
    outcome = receive_client_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    // Gestione della scelta dell'utente
    switch (choice) {
    case 1:
      // Creazione di un Nuovo Ticket
      outcome = handle_ticket_creation(socket, current_user->user_id);
      break;
    case 2:
      // Consultazione Ticket (richiama il sottomenu di consultazione)
      outcome = handle_ticket_consultation_menu(socket, current_user->user_id, current_user->type);
      break;
    case 3:
      // Logout (ritorna NEGATIVE per segnalare alla sessione di tornare al menu auth)
      return NEGATIVE;
    default:
      // Scelta non valida, continua il ciclo
      continue;
    }

    // Se si è verificato un errore critico di comunicazione, esci immediatamente
    if (outcome == ERROR) {
      return ERROR;
    }
  }
}

OperationOutcome handle_agent_menu(int socket, User *current_user) {
  OperationOutcome outcome;
  int choice;

  while (true) {

    // Attendi la scelta
    outcome = receive_client_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    switch (choice) {
    case 1:
      // Visualizza ticket (Menu consultazione filtrato per AGENT)
      outcome = handle_ticket_consultation_menu(socket, current_user->user_id, AGENT);
      break;
    case 2:
      // Gestione Ticket (Menu modifica stato/priorità)
      outcome = handle_ticket_management_menu(socket, current_user->user_id);
      break;
    case 3:
      // Visualizza ticket liberi (Non assegnati)
      outcome = handle_ticket_list_searching_by_unassigned(socket);
      break;
    case 4:
      // Assegna Ticket a se stesso (Presa in carico)
      outcome = handle_ticket_assignment(socket, current_user->user_id);
      break;
    case 5:
      // Logout: Ritorna NEGATIVE per segnalare alla sessione di tornare al menu auth
      return NEGATIVE;
    default:
      // Input non valido, ricomincia il ciclo
      continue;
    }

    // Se si verifica un errore critico, usciamo immediatamente
    if (outcome == ERROR) {
      return ERROR;
    }
  }
}

OperationOutcome handle_ticket_consultation_menu(int socket,
                                                 unsigned int current_user_id,
                                                 UserType user_type) {
  OperationOutcome outcome;
  int choice;

  while (true) {
    // Attendi la scelta
    outcome = receive_client_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    switch (choice) {
    case 1:
      // Ricerca ticket per ID
      outcome = handle_ticket_searching_by_id(socket, current_user_id, user_type);
      break;
    case 2:
      // Ricerca ticket per sottostringa nel titolo
      outcome = handle_ticket_list_searching_by_field(socket, current_user_id,
                                                      user_type, SUBSTRING);
      break;
    case 3:
      // Ricerca ticket per data
      outcome = handle_ticket_list_searching_by_field(socket, current_user_id,
                                                      user_type, DATE);
      break;
    case 4:
      // Stampa i ticket in base alla priorità
      outcome = handle_ticket_list_sorting(socket, current_user_id, user_type,
                                           PRIORITY);
      break;
    case 5:
      // Stampa tutti i ticket dell'utente
      outcome = handle_ticket_list_sorting(socket, current_user_id, user_type, NULL);
      break;
    case 6:
      // Torna al menu precedente
      return POSITIVE;
    default:
      // Scelta non valida, ricomincia il ciclo
      continue;
    }
    // Se si verifica un errore critico di comunicazione, esci immediatamente
    if (outcome == ERROR) {
      return ERROR;
    }
    // Altri outcome (NEGATIVE/POSITIVE dalle funzioni chiamate) permettono 
    // di restare in questo menu per ulteriori consultazioni.
  }
}

OperationOutcome handle_ticket_management_menu(int socket,
                                               unsigned int current_user_id) {
  OperationOutcome outcome;
  int choice;

  while (true) {

    // Attesa della scelta
    outcome = receive_client_choice(socket, &choice);
    if (outcome != POSITIVE) {
      return outcome;
    }

    switch (choice) {
    case 1:
      // Modifica priorità di un ticket
      outcome = handle_ticket_modification(socket, current_user_id, PRIORITY);
      break;
    case 2:
      // Modifica stato di un ticket
      outcome = handle_ticket_modification(socket, current_user_id, STATUS);
      break;
    case 3:
      // Torna al menu principale
      return POSITIVE;
    default:
      // Scelta non valida, ricomincia il ciclo
      continue;
    }

    // Se si verifica un errore critico di comunicazione, esci immediatamente
    if (outcome == ERROR) {
      return ERROR;
    }
    // Altri outcome (NEGATIVE/POSITIVE dalle funzioni chiamate) permettono 
    // di restare in questo menu per ulteriori modifiche.
  }
}

// ======================= Funzioni di supporto statiche ======================= 

// Ricezione della scelta del client
static OperationOutcome receive_client_choice(int socket, int *choice) {
  CommunicationStatus comm_status = receive_int(socket, choice);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  return POSITIVE;
}

// Gestione del segnale SIGCHLD per evitare processi zombie
static void handle_sigchild(int sig) {
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0) {
    ; // Continua a raccogliere i processi figli terminati
  }
  // Ripristina errno dopo la gestione del segnale
  errno = saved_errno;
}
