#ifndef COMMON_H
#define COMMON_H


// Dimensione buffer generico
#define BUFFER_SIZE 4096

// Dimensioni buffer per i campi ticket
#define TITLE_BUFFER_SIZE 64
#define DESC_BUFFER_SIZE 2048
#define DATE_BUFFER_SIZE 12

// Dimensioni massime per i campi ticket (sottraendo il carattere di terminazione '\0')
#define TITLE_MAX_LEN    (TITLE_BUFFER_SIZE - 1)
#define DESC_MAX_LEN     (DESC_BUFFER_SIZE - 1)
#define DATE_MAX_LEN     (DATE_BUFFER_SIZE - 1)   // Formato: GG/MM/AAAA

// Dimensioni buffer per i campi user
#define USERNAME_BUFFER_SIZE 32
#define PASSWORD_BUFFER_SIZE 64
#define EMAIL_BUFFER_SIZE    128

// Dimensioni massime per i campi user (sottraendo il carattere di terminazione '\0')
#define USERNAME_MAX_LEN (USERNAME_BUFFER_SIZE - 1)
#define PASSWORD_MAX_LEN (PASSWORD_BUFFER_SIZE - 1)
#define EMAIL_MAX_LEN    (EMAIL_BUFFER_SIZE - 1)

#endif // COMMON_H