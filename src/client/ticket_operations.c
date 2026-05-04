#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/client/ticket_operations.h"
#include "../../include/utils/communication.h"
#include "../../include/utils/config.h"

// Dichiarazione dei prototipi
static OperationOutcome receive_ticket(int socket);
static OperationOutcome receive_ticket_list(int socket);
static void clear_screen();

OperationOutcome ticket_creation(int socket) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  char title[TITLE_BUFFER_SIZE];
  char description[DESC_BUFFER_SIZE];

  // Raccolta dati: Titolo
  printf("Inserisci il titolo del ticket (MAX %d caratteri): \n",
         TITLE_MAX_LEN);
  input_outcome = get_title_stdin(title);

  // Raccolta dati: Descrizione
  if (input_outcome == INPUT_SUCCESS) {
    printf("Inserisci la descrizione del ticket (MAX %d caratteri): \n",
           DESC_MAX_LEN);
    input_outcome = get_description_stdin(description);
  }

  // Handshake di disponibilità dati
  availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // check immediato
  if (availability != DATA_AVAILABLE)
    return NEGATIVE;
  // Invio dei dati (Titolo e Descrizione)
  comm_status = send_string(socket, title);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  comm_status = send_string(socket, description);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Attendi l'esito della creazione dal server
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Gestione dell'esito
  if (op_outcome == POSITIVE) {
    printf("Ticket creato con successo!\n");
    // Ricevi e stampa il ticket appena creato
    op_outcome = receive_ticket(socket);
  } else {
    fprintf(
        stderr,
        "Errore di sistema lato server (Errore interno o dati non validi).\n");
  }
  return op_outcome;
}

OperationOutcome ticket_searching_by_id(int socket) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  unsigned int ticket_id;

  printf("Inserisci l'ID del ticket da cercare:\n");
  input_outcome = get_unsigned_int_stdin(&ticket_id);

  // Handshake di disponibilità dati
  availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // check immediato
  if (availability != DATA_AVAILABLE)
    return NEGATIVE;

  // Invio dell'ID del ticket
  comm_status = send_unsigned_int(socket, ticket_id);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Attesa dell'esito della ricerca
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Ricevi il risultato
  if (op_outcome == POSITIVE) {
    // Se trovato, il server invia i dati del ticket subito dopo
    op_outcome = receive_ticket(socket);
  } else if (op_outcome == NEGATIVE) {
    printf("Nessun ticket trovato con ID %d.\n", ticket_id);
  } else {
    fprintf(stderr, "Errore di sistema lato server durante la ricerca.\n");
  }
  return op_outcome;
}

OperationOutcome ticket_list_searching_by_field(int socket, const char *field) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  char search_field[TITLE_BUFFER_SIZE]; // Usato sia per titolo che per data
                                        // (TITLE_BUFFER_SIZE >>
                                        // DATE_BUFFER_SIZE)

  // Raccolta dati in base al campo
  if (strcmp(field, SUBSTRING) == 0) {
    printf("Inserisci una parte del titolo da cercare (MAX %d caratteri):\n",
           TITLE_MAX_LEN);
    input_outcome = get_title_stdin(search_field);
  } else if (strcmp(field, DATE) == 0) {
    printf("Inserisci la data (GG/MM/AAAA):\n");
    input_outcome = get_date_stdin(search_field);
  } else {
    // Caso di errore logico interno (modalità non supportata)
    return ERROR;
  }

  // Handshake di disponibilità dati
  availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Se l'input non è valido o è stato annullato, ci fermiamo
  if (availability != DATA_AVAILABLE)
    return NEGATIVE;

  // Inviamo la stringa cercata (titolo o data) al server
  comm_status = send_string(socket, search_field);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Attesa esito ricerca (Se esistono ticket corrispondenti)
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Gestione dell'esito
  if (op_outcome == POSITIVE) {
    printf("Ticket trovati:\n");
    // Ricezione della lista effettiva
    op_outcome = receive_ticket_list(socket);
  } else if (op_outcome == NEGATIVE) {
    printf("Nessun ticket trovato con i criteri specificati.\n");
  } else {
    fprintf(stderr, "Errore di sistema lato server durante la ricerca.\n");
  }

  return op_outcome;
}

OperationOutcome ticket_list_sorting(int socket) {
  CommunicationStatus comm_status;
  OperationOutcome outcome;

  //  Attesa dell'esito dell'operazione dal server
  comm_status = receive_outcome(socket, &outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Ricezione Dati
  if (outcome == POSITIVE) {
    printf("Inizio stampa dei ticket trovati:\n");
    outcome = receive_ticket_list(socket);
  } else if (outcome == NEGATIVE) {
    printf("Nessun Ticket trovato.\n");
  } else {
    fprintf(stderr,
            "Errore di sistema lato server durante il recupero della lista.\n");
  }

  return outcome;
}

OperationOutcome ticket_assignment(int socket) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  unsigned int ticket_id;

  printf("Inserisci l'ID del ticket da assegnare:\n");
  input_outcome = get_unsigned_int_stdin(&ticket_id);

  // Handshake di disponibilità dati
  availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Invio dell'ID del ticket
  comm_status = send_unsigned_int(socket, ticket_id);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Attesa esito dal server
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Gestione dell'esito
  if (op_outcome == POSITIVE) {
    printf("Ticket assegnato con successo!\n");
  } else if (op_outcome == NEGATIVE) {
    printf("Nessun ticket trovato con ID %d o già assegnato.\n", ticket_id);
  } else {
    fprintf(stderr, "Errore di sistema lato server durante l'assegnazione.\n");
  }
  return op_outcome;
}

OperationOutcome ticket_modification(int socket, const char *field) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome op_outcome;
  InputOutcome input_outcome;
  unsigned int ticket_id;
  int new_value;

  // Raccolta dati utente: ID Ticket
  printf("Inserisci ID del ticket: ");
  input_outcome = get_unsigned_int_stdin(&ticket_id);

  // Raccolta dati utente: Nuovo Valore
  if (input_outcome == INPUT_SUCCESS) {
    if (strcmp(field, PRIORITY) == 0) {
      printf("Inserisci nuova priorità (1-BASSO, 2-MEDIO, 3-ALTO): ");
      input_outcome = get_priority_stdin(&new_value);
    } else if (strcmp(field, STATUS) == 0) {
      printf("Inserisci nuovo stato (1-APERTO, 2-IN LAVORAZIONE, 3-CHIUSO): ");
      input_outcome = get_status_stdin(&new_value);
    } else {
      fprintf(stderr, "Errore interno client: Campo non gestito '%s'\n", field);
      return ERROR;
    }
  }

  // Handshake di disponibilità dati
   availability = (input_outcome == INPUT_SUCCESS) ? DATA_AVAILABLE : DATA_UNAVAILABLE;
  comm_status = send_availability(socket, availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Invio ID Ticket
  comm_status = send_unsigned_int(socket, ticket_id);
  if (comm_status != COMM_SUCCESS)
    return handle_communication_signal(comm_status, __func__);

  // Verifica permessi d'accesso e esistenza ticket
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS)
    return handle_communication_signal(comm_status, __func__);

  if (op_outcome != POSITIVE) {
    if (op_outcome == NEGATIVE) {
      printf("Operazione negata: Ticket inesistente o non autorizzato.\n");
    } else {
      fprintf(stderr, "Errore critico di sistema lato server.\n");
    }
    return NEGATIVE;
  }

  // Invio del nuovo valore
  comm_status = send_int(socket, new_value);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Esito finale della modifica
  comm_status = receive_outcome(socket, &op_outcome);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Gestione differenziata dell'esito
  if (op_outcome == POSITIVE) {
    printf("Modifica completata con successo!\n");
  } else if (op_outcome == NEGATIVE) {
    printf("Modifica fallita: Ticket non trovato o autorizzazioni "
           "insufficienti.\n");
  } else {
    fprintf(stderr,
            "Errore critico lato server: Salvataggio modifiche fallito.\n");
  }
  return op_outcome;
}

// ======================== Funzioni di supporto static =========================

// Ricezione e stampa di un singolo ticket
static OperationOutcome receive_ticket(int socket) {
  CommunicationStatus comm_status;
  char buffer[BUFFER_SIZE];

  comm_status = receive_string(socket, buffer, BUFFER_SIZE);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  printf("%s", buffer);
  return POSITIVE;
}

// Ricezione e stampa di una lista di ticket
static OperationOutcome receive_ticket_list(int socket) {
  CommunicationStatus comm_status;
  OperationOutcome outcome;
  int count;

  // Ricezione del numero totale di ticket
  comm_status = receive_int(socket, &count);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  printf("Numero di ticket trovati: %d\n", count);

  // Ciclo di ricezione
  for (int i = 0; i < count; i++) {
    outcome = receive_ticket(socket);
    if (outcome != POSITIVE) {
      return outcome;
    }

    // Gestione interazione utente e sincronizzazione
    if (i < count - 1) { // Chiedi di continuare solo se non è l'ultimo
      printf("\nPremi INVIO per visualizzare il prossimo ticket...");
      while (getchar() != '\n')
        ;
    }

    // Invio ACK per sincronizzazione
    comm_status = send_string(socket, "ACK");
    if (comm_status != COMM_SUCCESS) {
      return handle_communication_signal(comm_status, __func__);
    }
  }

  printf("\nVisualizzazione completata.\n");
  return POSITIVE;
}

// Pulizia schermo del terminale
static void clear_screen() { system("clear"); }