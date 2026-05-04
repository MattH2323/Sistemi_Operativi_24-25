#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/client/input_validation.h"
#include "../../include/utils/config.h"


// Dichiarazione dei prototipi
static void clear_input_buffer();
static bool is_email_format_valid(const char *email);
static bool is_date_format_valid(const char *date);

InputOutcome get_int_stdin(int *value) {
  char buffer[BUFFER_SIZE];
  char *end_ptr;
  char *newline;
  long val;

  while (true) {
    // Lettura dalla stdin
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      // Controllo errore I/O o EOF
      if (feof(stdin))
        return INPUT_EOF;
      perror("Errore I/O");
      return INPUT_ERROR;
    }

    // Cerca il carattere di newline (ossia [invio])
    newline = strchr(buffer, '\n');

    // Se manca \n e il buffer è pieno, l'input è troppo lungo (NON dovrebbe mai
    // succedere vista la dimensione del buffer)
    if (newline == NULL && strlen(buffer) == BUFFER_SIZE - 1) {
      fprintf(stderr, "Errore: Input troppo lungo. Riprova:\n");
      clear_input_buffer();
      continue;
    }
    // Rimozione di \n
    if (newline) {
      *newline = '\0';
    }
    // Controllo se l'input è ESC + ENTER (Richiesta di cancellazione)
    if (buffer[0] == ESCAPE_CHAR && buffer[1] == '\0') {
      return INPUT_CANCEL;
    }
    // Conversione stringa a long
    errno = 0;
    val = strtol(buffer, &end_ptr, 10);

    // Controlli di validità
    if (end_ptr == buffer) {
      fprintf(stderr, "Errore: Numero non riconosciuto. Riprova:\n");
    }
    // Controllo caratteri non numerici
    else if (*end_ptr != '\0') {
      fprintf(stderr, "Errore: Input contiene caratteri non validi. Riprova:\n");
    } else if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
      // Controllo overflow/underflow di int, ERANGE è settato da strtol in caso
      // di overflow/underflow di tipo long
      fprintf(stderr, "Errore: Numero fuori range (max %d). Riprova:\n", INT_MAX);
    } else {
      // Assegna il valore convertito e ritorna successo
      *value = (int)val;
      return INPUT_SUCCESS;
    }
  }
}

InputOutcome get_unsigned_int_stdin(unsigned int *value) {
  char buffer[BUFFER_SIZE];
  char *endptr;
  char *ptr;
  unsigned long val;
  InputOutcome input_outcome;

  while (true) {
    // Usa la funzione sicura per leggere la stringa grezza
    input_outcome = get_string_stdin(buffer, sizeof(buffer));
    if (input_outcome != INPUT_SUCCESS) {
      return input_outcome;
    }
    ptr = buffer;
    // Salta spazi e tabulazioni iniziali
    while (*ptr == ' ' || *ptr == '\t') {
      ptr++;
    }
    // Controllo input vuoto
    if (*ptr == '\0') {
      fprintf(stderr, "Errore: Input vuoto. Riprova: ");
      continue;
    }
    // Controllo manuale del segno meno
    if (*ptr == '-' || *ptr == '+') {
      fprintf(stderr, "Errore: Inserire solo numeri unsigned.\n");
      continue;
    }
    // Conversione stringa a unsigned long
    errno = 0;
    val = strtoul(ptr, &endptr, 10);

    // Controlli di validità
    if (endptr == ptr) {
      fprintf(stderr, "Errore: Numero non riconosciuto. Riprova:\n");
    } else if (*endptr != '\0') {
      fprintf(stderr, "Errore: Input contiene caratteri non validi. Riprova:\n");
    } else if (errno == ERANGE || val > UINT_MAX) {
      fprintf(stderr, "Errore: Numero fuori range (max %u). Riprova:\n", UINT_MAX);
    } else {
      *value = (unsigned int)val;
      return INPUT_SUCCESS;
    }
  }
}

InputOutcome get_string_stdin(char *buffer, int buffer_size) {
  char *newline;

  // Controllo dimensione buffer
  if (buffer_size <= 1)
    return INPUT_ERROR;

  while (true) {
    // Lettura sicura da stdin con fgets
    if (fgets(buffer, buffer_size, stdin) == NULL) {
      if (feof(stdin))
        return INPUT_EOF;
      perror("Errore I/O");
      return INPUT_ERROR;
    }

    // Cerca il carattere di newline
    newline = strchr(buffer, '\n');

    if (newline != NULL) {      
      // Controllo Input vuoto (solo invio)
      if (newline == buffer) {
        fprintf(stderr, "Errore: Input vuoto. Riprova: ");
        continue;
      }

      // Controllo ESC + ENTER
      if (buffer[0] == ESCAPE_CHAR && (buffer + 1) == newline) {
        return INPUT_CANCEL;
      }

      // Rimuovi il newline e ritorna successo
      *newline = '\0';
      return INPUT_SUCCESS;
      
    } else {
      // Input troppo lungo per il buffer, pulisco
      fprintf(stderr, "Errore: Input troppo lungo (max %d). Riprova: ", buffer_size - 1);
      
      clear_input_buffer(); 
    }
  }
}

InputOutcome get_mail_stdin(char *mail) {
  while (true) {
    InputOutcome input_outcome = get_string_stdin(mail, EMAIL_BUFFER_SIZE);
    if (input_outcome != INPUT_SUCCESS) {
      return input_outcome;
    }
    if (is_email_format_valid(mail)) {
      return INPUT_SUCCESS;
    } else {
      printf("Formato email invalido. Riprova:\n");
    }
  }
}

InputOutcome get_date_stdin(char *date) {
  while (true) {
    InputOutcome input_outcome = get_string_stdin(date, DATE_BUFFER_SIZE);
    if (input_outcome != INPUT_SUCCESS) {
      return input_outcome;
    }
    if (is_date_format_valid(date)) {
      return INPUT_SUCCESS;
    } else {
      printf("Formato data invalido. Riprova (GG/MM/AAAA):\n");
    }
  }
}

InputOutcome get_status_stdin(int *status_stdin) {
  while (true) {
    InputOutcome input_outcome = get_int_stdin(status_stdin);
    if (input_outcome != INPUT_SUCCESS) {
      return input_outcome;
    }
    if (*status_stdin == 1 || *status_stdin == 2 || *status_stdin == 3)
      return INPUT_SUCCESS;
    else
      printf("Stato inserito non valido. Riprova: (1-APERTO, 2-IN LAVORAZIONE, 3-CHIUSO)\n");
  }
}

InputOutcome get_priority_stdin(int *priority) {
  while (true) {
    InputOutcome input_outcome = get_int_stdin(priority);
    if (input_outcome != INPUT_SUCCESS) {
      return input_outcome;
    }
    if (*priority == 1 || *priority == 2 || *priority == 3) {
      return INPUT_SUCCESS;
    } else {
      printf("Priorità inserita invalida. Riprova (1-BASSO, 2-MEDIO, 3-ALTO)\n");
    }
  }
}

InputOutcome get_title_stdin(char *title) {
  return get_string_stdin(title, TITLE_BUFFER_SIZE);
}

InputOutcome get_description_stdin(char *description) {
  return get_string_stdin(description, DESC_BUFFER_SIZE);
}

InputOutcome get_username_stdin(char *username) {
  return get_string_stdin(username, USERNAME_BUFFER_SIZE);
}

InputOutcome get_password_stdin(char *password) {
  return get_string_stdin(password, PASSWORD_BUFFER_SIZE);
}

// ======================== Funzioni di supporto static =========================

// Pulizia del buffer di input stdin
static void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

// Verifica formato email di base [*@*.*]
static bool is_email_format_valid(const char *email) {
  const char *at_ptr = strchr(email, '@');   // Trova la prima @
  const char *dot_ptr = strrchr(email, '.'); // Trova l'ultimo .

  return (at_ptr && dot_ptr && at_ptr > email && dot_ptr > at_ptr + 1 &&
          dot_ptr[1] != '\0');
}

// Verifica formato data GG/MM/AAAA
static bool is_date_format_valid(const char *date) {
  if (strlen(date) != 10)
    return false;
  for (int i = 0; i < 10; i++) {
    if (i == 2 || i == 5) {
      if (date[i] != '/')
        return false;
    } else {
      if (!isdigit(date[i]))
        return false;
    }
  }
  int day, month, year;
  // Estrazione di giorno, mese, anno
  time_t real_time = sscanf(date, "%d/%d/%d", &day, &month, &year);

  // Controllo anno (limitiamo tra 1900 e 2100)
  if (year < 1900 || year > 2100)
    return false;

  // Controllo mese
  if (month < 1 || month > 12)
    return false;

  // Controllo giorni base
  if (day < 1 || day > 31)
    return false;

  // Controllo giorni per mese
  if (month == 4 || month == 6 || month == 9 || month == 11) {
    if (day > 30)
      return false;
  }
  return true;
}
