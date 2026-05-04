#ifndef INPUT_VALIDATION_H
#define INPUT_VALIDATION_H

// Possibili valori di ritorno dalle funzioni di input
typedef enum InputOutcome{
    INPUT_CANCEL = -3,  // causato da inserimento ESC + ENTER, Input cancellato dall'utente
    INPUT_EOF,          // Fine del file o input terminato
    INPUT_ERROR,        // Errore durante la lettura dell'input
    INPUT_SUCCESS       // Input letto con successo
} InputOutcome;

// Valore del carattere di ESCAPE
#define ESCAPE_CHAR 27

// Lettura di un intero da stdin
InputOutcome get_int_stdin(int* value);

// Lettura di un intero non negativo da stdin
InputOutcome get_unsigned_int_stdin(unsigned int* value);

// Lettura di una stringa da stdin
InputOutcome get_string_stdin(char *buffer, int buffer_size);

// Lettura di un username da stdin con lunghezza max = USERNAME_MAX_LEN
InputOutcome get_username_stdin(char* username);

// Lettura di una password da stdin con lunghezza max = PASSWORD_MAX_LEN
InputOutcome get_password_stdin(char* password);

// Lettura di una mail da stdin con lunghezza max = EMAIL_MAX_LEN
InputOutcome get_mail_stdin(char* mail);

// Lettura di una data da stdin con lunghezza max = DATE_MAX_LEN
InputOutcome get_title_stdin (char* title);

// Lettura di una descrizione da stdin con lunghezza max = DESC_MAX_LEN
InputOutcome get_description_stdin(char* description);

// Lettura di una data da stdin con formato GG/MM/AAAA
InputOutcome get_date_stdin(char*date);

// Prende uno stato da stdin con validazione (0-OPEN, 1-IN PROGRESS, 2-CLOSED)
InputOutcome get_status_stdin(int * status_stdin);

// Prende una priorità da stdin con validazione (0-LOW, 1-MEDIUM, 2-HIGH)
InputOutcome get_priority_stdin (int * priority);


#endif