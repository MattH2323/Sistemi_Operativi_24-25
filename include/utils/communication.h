#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdlib.h> 

// ========== CONNECTION MACROS ==========

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 12345

// "client" || "agent" < USER_TYPE_LENGTH
#define USER_TYPE_LENGTH 8

// User types per la comunicazione
#define CLIENT_STRING "CLIENT"
#define AGENT_STRING "AGENT"
#define SUBSTRING "SUBSTRING"
#define DATE "DATE"
#define PRIORITY "PRIORITY"
#define STATUS "STATUS"

// Risultato delle operazioni di comunicazione
typedef enum CommunicationStatus {
  COMM_CLOSED = -3, // Connessione chiusa
  COMM_ERROR,       // Errore generico (I/O o logico)
  COMM_SUCCESS,     // Operazione completata con successo
} CommunicationStatus;

// Definzione tipo di ritorno per operazioni della business logic
typedef enum OperationOutcome { NEGATIVE, POSITIVE, ERROR } OperationOutcome;

// Definizione stati di disponibilità dati
typedef enum DataAvailability {
  DATA_UNAVAILABLE,
  DATA_AVAILABLE
} DataAvailability;

// Ricevi un intero
CommunicationStatus receive_int(int socket, int *choice);

// Ricevi un intero senza segno
CommunicationStatus receive_unsigned_int(int socket, unsigned int *value);

// Ricevi una stringa
CommunicationStatus receive_string(int socket, char *buffer,
                                   int buffer_capacity);

// Ricevi un tipo OperationOutcome
CommunicationStatus receive_outcome(int socket, OperationOutcome *outcome);

// Ricevi un tipo DataAvailability
CommunicationStatus receive_availability(int socket,
                                         DataAvailability *availability);

// Invio di un intero
CommunicationStatus send_int(int socket, int value);

// Invio di un intero senza segno
CommunicationStatus send_unsigned_int(int socket, unsigned int value);

// Invio di una stringa
CommunicationStatus send_string(int socket, const char *string_ptr);

// Invio di un tipo OperationOutcome
CommunicationStatus send_outcome(int socket, OperationOutcome outcome);

// Invio di un tipo DataAvailability
CommunicationStatus send_availability(int socket,
                                      DataAvailability availability);

// Gestione dei segnali di comunicazione
OperationOutcome handle_communication_signal(CommunicationStatus comm_status,
                                             const char *function_name);

#endif