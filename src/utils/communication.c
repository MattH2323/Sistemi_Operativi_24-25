#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../../include/utils/communication.h"

CommunicationStatus receive_int(int socket, int *buffer) {
  int net_int;
  ssize_t bytes_read;
  size_t total_read = 0;
  size_t size_to_read = sizeof(int);
  char *buffer_ptr = (char *)&net_int; // puntatore per memorizzare i byte
  while (total_read < size_to_read) {
    bytes_read =
        read(socket, buffer_ptr + total_read, size_to_read - total_read);
    if (bytes_read < 0) {
      if (errno == EINTR)
        continue;
      perror("receive_int: read failed");
      return COMM_ERROR;
    }
    if (bytes_read == 0) {
      fprintf(stderr, "Errore: connessione chiusa durante la ricezione di un int\n");
      return COMM_CLOSED;
    }
    total_read += bytes_read;
  }
  *buffer = ntohl(net_int);
  return COMM_SUCCESS;
}

CommunicationStatus receive_unsigned_int(int socket, unsigned int *buffer) {
  unsigned int net_int;
  ssize_t bytes_read;
  size_t total_read = 0;
  size_t size_to_read = sizeof(unsigned int);

  char *buffer_ptr = (char *)&net_int;
  
  // Leggiamo fino a quando non abbiamo letto tutti i byte necessari
  while (total_read < size_to_read) {
    bytes_read =
        read(socket, buffer_ptr + total_read, size_to_read - total_read);
    // Gestione degli errori di lettura
    if (bytes_read < 0) {
      if (errno == EINTR)
        continue;
      perror("receive_unsigned_int: read failed");
      return COMM_ERROR;
    }
    // Gestione della chiusura della connessione
    if (bytes_read == 0) {
      fprintf(stderr,
              "Errore: connessione chiusa durante la ricezione di un unsigned int.\n");
      return COMM_CLOSED;
    }
    total_read += bytes_read;
  }
  // Convertiamo da network byte order a host byte order
  *buffer = ntohl(net_int);

  return COMM_SUCCESS;
}

CommunicationStatus receive_string(int socket, char *buffer,
                                   int buffer_capacity) {
  int bytes_read;
  int total_read = 0;

  // Leggiamo byte per byte fino alla lunghezza massima
  while (total_read < buffer_capacity - 1) {
    bytes_read = read(socket, buffer + total_read, 1);

    if (bytes_read < 0) {
      perror("receive_string: read failed\n");
      return COMM_ERROR;
    }

    if (bytes_read == 0) {
      buffer[total_read] = '\0';
      fprintf(stderr,
              "Errore: connessione chiusa durante la ricezione di una stringa.\n");
      return COMM_CLOSED;
    }

    // Controlliamo se abbiamo ricevuto il terminatore di stringa
    if (buffer[total_read] == '\0') {
      return COMM_SUCCESS;
    }

    total_read++;
  }

  // Se usciamo dal while, abbiamo riempito il buffer
  buffer[buffer_capacity - 1] = '\0';
  fprintf(stderr, "Errore: la stringa ricevuta è troppo lunga!\n");
  return COMM_ERROR; // O COMM_OVERFLOW se vuoi gestire il caso specifico
}

CommunicationStatus receive_outcome(int socket, OperationOutcome *outcome) {
  int int_value;
  CommunicationStatus comm_status = receive_int(socket, &int_value);
  *outcome = (OperationOutcome)int_value;
  return comm_status;
}

CommunicationStatus receive_availability(int socket,
                                         DataAvailability *availability) {
  int int_value;
  CommunicationStatus comm_status = receive_int(socket, &int_value);
  *availability = (DataAvailability)int_value;
  return comm_status;
}

CommunicationStatus send_int(int socket, int value) {
  // Converti a network byte order (assicura la compatibilità tra architetture diverse)
  int net_value = htonl(value);
  int bytes_sent;
  int total_sent = 0;
  int bytes_to_send = sizeof(int);
  const char *int_to_send = (const char *)&net_value;

  while (total_sent < bytes_to_send) {
    bytes_sent =
        write(socket, int_to_send + total_sent, bytes_to_send - total_sent);
    if (bytes_sent < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("send_int: write failed\n");
      return COMM_ERROR;
    }
    total_sent += bytes_sent;
  }
  return COMM_SUCCESS;
}

CommunicationStatus send_unsigned_int(int socket, unsigned int value) {
  // Converti a network byte order (assicura la compatibilità tra architetture diverse)
  unsigned int net_value = htonl(value);

  ssize_t bytes_sent; // ssize_t è il tipo standard restituito da write
  int total_sent = 0;
  int bytes_to_send = sizeof(unsigned int);

  const char *data_to_send = (const char *)&net_value;

  while (total_sent < bytes_to_send) {
    bytes_sent =
        write(socket, data_to_send + total_sent, bytes_to_send - total_sent);

    if (bytes_sent < 0) {

      if (errno == EINTR) {
        // Interruzione di segnale, riprova
        continue;
      }
      perror("Errore di comunicazione: write failed\n");
      return COMM_ERROR;
    }
    total_sent += bytes_sent;
  }
  return COMM_SUCCESS;
}

CommunicationStatus send_string(int socket, const char *string) {
  int length_to_send = strlen(string) + 1;
  int bytes_sent;
  int length_sent = 0;
  while (length_sent < length_to_send) {
    bytes_sent =
        write(socket, string + length_sent, length_to_send - length_sent);
    if (bytes_sent < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("Errore di comunicazione: write failed\n");
      return COMM_ERROR;
    }
    length_sent += bytes_sent;
  }
  return COMM_SUCCESS;
}

CommunicationStatus send_outcome(int socket, OperationOutcome outcome) {
  return send_int(socket, (int)outcome);
}

CommunicationStatus send_availability(int socket,
                                      DataAvailability availability) {
  return send_int(socket, (int)availability);
}

OperationOutcome handle_communication_signal(CommunicationStatus comm_status,
                                             const char *function_name) {
  switch (comm_status) {
  // Comunicazione avvenuta con successo -> Operazione positiva
  case COMM_SUCCESS:
    return POSITIVE;
  // Comunicazione chiusa o errore -> Operazione negativa
  case COMM_CLOSED:
    fprintf(stderr, "[Connessione Chiusa] In: %s\n", function_name);
    return NEGATIVE;
  // Errore di comunicazione -> Errore di sistema
  case COMM_ERROR:
    fprintf(stderr, "[Errore Comunicazione] In: %s\n", function_name);
    return ERROR;
  default:
    fprintf(stderr, "[Errore Interno] In: %s\n", function_name);
    return ERROR;
  }
}