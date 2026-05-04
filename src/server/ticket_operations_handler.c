#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/json/cjson_ticket.h"
#include "../../include/server/ticket_operations_handler.h"
#include "../../include/utils/communication.h"

// Dichiarazione dei prototipi
static OperationOutcome send_ticket(int socket, Ticket *ticket);
static OperationOutcome send_ticket_list(int socket, TicketList *list);

OperationOutcome handle_ticket_creation(int socket,
                                        unsigned int current_user_id) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome outcome = POSITIVE;
  TicketList *tickets;
  Ticket *new_ticket;
  char title[TITLE_BUFFER_SIZE];
  char description[DESC_BUFFER_SIZE];

  // Handshake di disponibilità dati
  comm_status = receive_availability(socket, &availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }
  // Ricezione dati del nuovo ticket
  comm_status = receive_string(socket, title, TITLE_BUFFER_SIZE);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Ricezione descrizione
  comm_status = receive_string(socket, description, DESC_BUFFER_SIZE);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Caricamento lista in modalità scrittura per poter salvare il nuovo ticket
  tickets = load_ticket_list("r+");
  if (tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }
  // Creazione del nuovo ticket
  new_ticket = create_ticket(title, description, current_user_id);
  if (new_ticket == NULL) {
    outcome = ERROR;
    goto send_result;
  }
  // Aggiunta del ticket alla lista
  add_ticket(tickets, new_ticket);
  // Salvataggio della lista aggiornata
  if (save_ticket_list(tickets) < 0) {
    outcome = ERROR;
    goto send_result;
  }

send_result:
  comm_status = send_outcome(socket, outcome);
  // Comunicazione fallita, pulizia memoria
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    return handle_communication_signal(comm_status, __func__);
  }
  // Invio del nuovo ticket creato
  if (outcome == POSITIVE && new_ticket != NULL) {
    OperationOutcome ticket_send_outcome = send_ticket(socket, new_ticket);
    if (ticket_send_outcome != POSITIVE) {
      fprintf(stderr, "Error: send_ticket failed in %s\n", __func__);
    }
  }
  // Pulizia finale
  clear_ticket_list(tickets);

  return outcome;
}

OperationOutcome handle_ticket_searching_by_id(int socket,
                                               unsigned int current_user_id,
                                               UserType user_type) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome outcome;
  unsigned int ticket_id;
  TicketList *tickets;
  Ticket *ticket_found;

  // Handshake di disponibilità dati
  comm_status = receive_availability(socket, &availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Ricezione ID Ticket
  comm_status = receive_unsigned_int(socket, &ticket_id);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Caricamento lista completa dei ticket
  tickets = load_ticket_list("r");
  if (tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  // Ricerca del ticket per ID
  ticket_found = get_ticket_by_id(tickets, ticket_id);
  if (ticket_found == NULL) {
    outcome = NEGATIVE;
  } else if (user_type == CLIENT && ticket_found->userId != current_user_id) {
    outcome = NEGATIVE;
  } else {
    outcome = POSITIVE;
  }

send_result:
  comm_status = send_outcome(socket, outcome);
  // Comunicazione fallita, pulizia memoria
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    return handle_communication_signal(comm_status, __func__);
  }
  // Invio del ticket trovato
  if (outcome == POSITIVE) {
    OperationOutcome ticket_send_outcome = send_ticket(socket, ticket_found);
    if (ticket_send_outcome != POSITIVE) {
      fprintf(stderr, "Error: send_ticket failed in %s\n", __func__);
    }
  }
  // Pulizia finale
  clear_ticket_list(tickets);
  return outcome;
}

OperationOutcome
handle_ticket_list_searching_by_field(int socket, unsigned int current_user_id,
                                      UserType user_type, const char *mode) {
  DataAvailability availability;
  CommunicationStatus comm_status;
  OperationOutcome outcome;
  char title[TITLE_BUFFER_SIZE];
  char date[DATE_BUFFER_SIZE];
  TicketList *tickets;
  TicketList *user_tickets;
  TicketList *tickets_found;

  // Handshake di disponibilità dati
  comm_status = receive_availability(socket, &availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // check immediato
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Ricezione del campo di ricerca
  if (strcmp(mode, SUBSTRING) == 0) {
    comm_status = receive_string(socket, title, TITLE_BUFFER_SIZE);
    if (comm_status != COMM_SUCCESS)
      return handle_communication_signal(comm_status, __func__);
  } else if (strcmp(mode, DATE) == 0) {
    comm_status = receive_string(socket, date, DATE_BUFFER_SIZE);
    if (comm_status != COMM_SUCCESS)
      return handle_communication_signal(comm_status, __func__);
  }

  // Caricamento lista completa dei ticket
  tickets = load_ticket_list("r");
  if (tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  // Filtra i ticket per user_id o agent_id
  if (user_type == CLIENT) {
    user_tickets = filter_by_user_id(tickets, current_user_id);
  } else if (user_type == AGENT) {
    user_tickets = filter_by_agent_id(tickets, current_user_id);
  }

  // Errore nel filtro
  if (user_tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  // Applica il filtro specifico (sottostringa titolo o data)
  if (strcmp(mode, SUBSTRING) == 0) {
    tickets_found = filter_by_title_substring(user_tickets, title);
  } else if (strcmp(mode, DATE) == 0) {
    tickets_found = filter_by_date(user_tickets, date);
  }
  clear_ticket_list_without_data(user_tickets);

  // Errore nel filtro
  if (tickets_found == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  outcome = (tickets_found->head) ? POSITIVE : NEGATIVE;

send_result:
  comm_status = send_outcome(socket, outcome);

  // Se la comunicazione fallisce, dobbiamo comunque pulire la memoria
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    clear_ticket_list_without_data(tickets_found);
    return handle_communication_signal(comm_status, __func__);
  }
  // Se tutto ok e outcome positivo, inviamo la lista
  if (outcome == POSITIVE) {
    outcome = send_ticket_list(socket, tickets_found);
  }
  // Pulizia finale
  clear_ticket_list(tickets);
  clear_ticket_list_without_data(tickets_found);

  return outcome;
}

OperationOutcome handle_ticket_list_searching_by_unassigned(int socket) {
  CommunicationStatus comm_status;
  OperationOutcome outcome;
  TicketList *tickets;
  TicketList *tickets_found;

  // Carica tutti i ticket
  tickets = load_ticket_list("r");
  if (tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  // Filtra la lista per i ticket con agent_id = 0 (non assegnati)
  tickets_found = filter_by_agent_id(tickets, 0);

  if (tickets_found == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  // Determina l'outcome: POSITIVE se ci sono ticket, NEGATIVE altrimenti.
  outcome = (tickets_found->head) ? POSITIVE : NEGATIVE;

send_result:
  // Invio risultato
  comm_status = send_outcome(socket, outcome);
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    clear_ticket_list_without_data(tickets_found);
    return handle_communication_signal(comm_status, __func__);
  }

  if (outcome == POSITIVE) {
    outcome = send_ticket_list(socket, tickets_found);
  }

  clear_ticket_list(tickets);
  clear_ticket_list_without_data(tickets_found);

  return outcome;
}

OperationOutcome handle_ticket_list_sorting(int socket,
                                            unsigned int current_user_id,
                                            UserType user_type,
                                            const char *sort_type) {
  OperationOutcome outcome;
  CommunicationStatus comm_status;
  TicketList *tickets;
  TicketList *tickets_found;

  // Carica tutti i ticket
  tickets = load_ticket_list("r");
  if (!tickets) {
    outcome = ERROR;
    goto send_result;
  }

  // Filtra i ticket per user_id o agent_id
  if (user_type == CLIENT) {
    tickets_found = filter_by_user_id(tickets, current_user_id);
  } else if (user_type == AGENT) {
    tickets_found = filter_by_agent_id(tickets, current_user_id);
  }

  // Verifica errori nel filtro
  if (!tickets_found) {
    outcome = ERROR;
    goto send_result;
  }

  // Ordina i ticket
  if (sort_type != NULL) {
    if (strcmp(sort_type, PRIORITY) == 0) {
      sort_tickets_by_priority(tickets_found);
    } else {
      outcome = ERROR;
      goto send_result;
    }
  }

  // Determina esito logico
  outcome = (tickets_found->head) ? POSITIVE : NEGATIVE;

send_result:
  // Invio risultato
  comm_status = send_outcome(socket, outcome);

  // Se la comunicazione fallisce, dobbiamo comunque pulire la memoria
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    clear_ticket_list_without_data(tickets_found);
    return handle_communication_signal(comm_status, __func__);
  }
  // Se tutto ok e outcome positivo, inviamo la lista
  if (outcome == POSITIVE) {
    outcome = send_ticket_list(socket, tickets_found);
  }
  // Pulizia finale (sicura perché i puntatori sono NULL o validi)
  clear_ticket_list(tickets);
  clear_ticket_list_without_data(tickets_found);

  return outcome;
}

OperationOutcome handle_ticket_modification(int socket,
                                            unsigned int current_user_id,
                                            const char *modify_type) {
  DataAvailability availability;
  OperationOutcome outcome;
  CommunicationStatus comm_status;
  TicketList *tickets;
  Ticket *ticket_found;
  unsigned int ticket_id;

  // Handshake di disponibilità dati
  comm_status = receive_availability(socket, &availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Se non vi sono i dati, ritorna negative
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Ricezione ID del ticket da modificare
  comm_status = receive_unsigned_int(socket, &ticket_id);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Caricamento lista in modalità scrittura per poter salvare le modifiche
  tickets = load_ticket_list("r+");
  if (tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }

  ticket_found = get_ticket_by_id(tickets, ticket_id);

  // Verifica permessi: il ticket deve esistere ed appartenere all'utente
  if (ticket_found == NULL || ticket_found->agentId != current_user_id) {
    outcome = NEGATIVE;
  } else {
    outcome = POSITIVE;
  }

  // Invio esito ricerca al client
  comm_status = send_outcome(socket, outcome);
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    return handle_communication_signal(comm_status, __func__);
  }

  // Se l'esito intermedio è negativo, non procedere con la modifica
  if (outcome != POSITIVE) {
    clear_ticket_list(tickets);
    return outcome;
  }

  // Modifica della priorità o dello stato
  if (strcmp(modify_type, PRIORITY) == 0) {
    // PRIORITY
    int new_priority;
    comm_status = receive_int(socket, &new_priority);
    if (comm_status != COMM_SUCCESS) {
      clear_ticket_list(tickets);
      return handle_communication_signal(comm_status, __func__);
    }
    set_ticket_priority(ticket_found, (Priority)new_priority);
  } else if (strcmp(modify_type, STATUS) == 0) {
    // STATUS
    int new_status;
    comm_status = receive_int(socket, &new_status);
    if (comm_status != COMM_SUCCESS) {
      clear_ticket_list(tickets);
      return handle_communication_signal(comm_status, __func__);
    }
    set_ticket_status(ticket_found, (Status)new_status);
  } else {
    // Tipo di modifica non riconosciuto
    outcome = ERROR;
  }

  // Salvataggio delle modifiche
  if (outcome == POSITIVE) {
    if (save_ticket_list(tickets) < 0) {
      fprintf(stderr, "%s: save_ticket_list failed\n", __func__);
      outcome = ERROR;
    }
  }

send_result:
  // Invio risultato finale dell'operazione
  comm_status = send_outcome(socket, outcome);
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    return handle_communication_signal(comm_status, __func__);
  }

  // Pulizia della memoria allocata
  clear_ticket_list(tickets);
  return outcome;
}

OperationOutcome handle_ticket_assignment(int socket,
                                          unsigned int current_user_id) {
  DataAvailability availability;
  OperationOutcome outcome = POSITIVE;
  CommunicationStatus comm_status;
  TicketList *tickets = NULL;
  Ticket *ticket_found = NULL;
  unsigned int ticket_id;

  // Handshake di disponibilità dati
  comm_status = receive_availability(socket, &availability);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Se non vi sono i dati, ritorna outcome
  if (availability != DATA_AVAILABLE) {
    return NEGATIVE;
  }

  // Ricezione ID del ticket da modificare
  comm_status = receive_unsigned_int(socket, &ticket_id);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }

  // Caricamento lista in modalità scrittura per poter salvare le modifiche
  tickets = load_ticket_list("r+");
  if (tickets == NULL) {
    outcome = ERROR;
    goto send_result;
  }
  // Ricerca del nodo tramite ID
  ticket_found = get_ticket_by_id(tickets, ticket_id);

  // Logica presa in carico: il ticket deve esistere ed essere libero (
  if (ticket_found == NULL || ticket_found->agentId != 0) {
    outcome = NEGATIVE;
  } else {
    set_ticket_agent(ticket_found, current_user_id);
    set_ticket_status(ticket_found, IN_PROGRESS);

    // Se il salvataggio fallisce, l'operazione deve essere considerata fallita.
    if (save_ticket_list(tickets) < 0) {
      outcome = ERROR;
    }
  }

send_result:
  // Invio risultato finale dell'operazione
  comm_status = send_outcome(socket, outcome);
  if (comm_status != COMM_SUCCESS) {
    clear_ticket_list(tickets);
    return handle_communication_signal(comm_status, __func__);
  }

  // Pulizia della memoria allocata
  clear_ticket_list(tickets);
  return outcome;
}

// ======================= Funzioni di supporto statiche =======================
// //

// Invio di una lista di ticket al client
static OperationOutcome send_ticket_list(int socket, TicketList *list) {
  CommunicationStatus comm_status;
  OperationOutcome outcome;

  // Controllo validità lista
  if (list == NULL || list->head == NULL) {
    return ERROR;
  }
  // Invia il numero totale di ticket
  comm_status = send_unsigned_int(socket, list->count);
  if (comm_status != COMM_SUCCESS) {
    return handle_communication_signal(comm_status, __func__);
  }
  // Invio sequenziale dei ticket
  for (unsigned int i = 0; i < list->count; i++) {
    Ticket *current_ticket = get_by_index(list, i);

    if (current_ticket == NULL) {
      return ERROR;
    }
    // Invio dati del ticket
    outcome = send_ticket(socket, current_ticket);
    if (outcome != POSITIVE) {
      return outcome;
    }
    // Attesa "ACK" (stringa di sincronizzazione)
    char ack_buffer[BUFFER_SIZE];
    comm_status = receive_string(socket, ack_buffer, sizeof(ack_buffer));
    if (comm_status != COMM_SUCCESS) {
      return handle_communication_signal(comm_status, __func__);
    }
  }
  return POSITIVE;
}

// Invio di un singolo ticket al client
static OperationOutcome send_ticket(int socket, Ticket *ticket) {
  CommunicationStatus comm_status;
  OperationOutcome outcome = POSITIVE;

  // Conversione del ticket in stringa
  char *ticket_info = ticket_to_string(ticket);
  if (ticket_info == NULL) {
    return ERROR;
  }
  // Invio della stringa contenente le info del ticket
  comm_status = send_string(socket, ticket_info);
  if (comm_status != COMM_SUCCESS) {
    outcome = handle_communication_signal(comm_status, __func__);
  }

  free(ticket_info);
  ticket_info = NULL;

  return outcome;
}