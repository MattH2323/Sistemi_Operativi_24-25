#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../../include/server/user_operations_handler.h"
#include "../../include/utils/communication.h"
#include "../../include/json/cjson_user.h"

OperationOutcome handle_login(int socket, User** user) {
    char username[USERNAME_BUFFER_SIZE];
    char password[PASSWORD_BUFFER_SIZE];
    OperationOutcome outcome = POSITIVE;
    CommunicationStatus comm_status;
    char user_type[USER_TYPE_LENGTH];
    UserList* users = NULL;

    // Handshake di disponibilità dati
    DataAvailability availability;
    comm_status = receive_availability(socket, &availability);
    if (comm_status != COMM_SUCCESS) {
      return handle_communication_signal(comm_status, __func__);
    }

    // check immediato
    if (availability != DATA_AVAILABLE) {
      return outcome;
    }

    // Ricezione delle credenziali
    comm_status = receive_string(socket, username, USERNAME_BUFFER_SIZE);
    if (comm_status != COMM_SUCCESS) {
        return handle_communication_signal(comm_status, __func__);
    }
    comm_status = receive_string(socket, password, PASSWORD_BUFFER_SIZE);
    if (comm_status != COMM_SUCCESS) {
        return handle_communication_signal(comm_status, __func__);
    }

    // Caricamento della lista utenti in modalità lettura
    users = load_user_list("r");

    if (users == NULL) {
        fprintf(stderr, "%s: load_user_list failed\n", __func__);
        outcome = ERROR;
        goto send_result;
    }

    // Autenticazione dell'utente
    *user = authenticate_client(users, username, password);
    
    // Determina l'esito in base al risultato dell'autenticazione
    outcome = (*user == NULL) ? NEGATIVE : POSITIVE;

send_result:
    // Invio dell'esito dell'autenticazione
    comm_status = send_outcome(socket, outcome);

    // Se la comunicazione fallisce, puliamo la memoria prima di gestire l'errore
    if (comm_status != COMM_SUCCESS) {
        clear_user_list(users);
        return handle_communication_signal(comm_status, __func__);
    }

    // Se l'autenticazione ha avuto successo, invia il tipo di utente
    if (outcome == POSITIVE) {
        const char *source_string = ((*user)->type == CLIENT) ?  CLIENT_STRING : AGENT_STRING;
        strncpy(user_type, source_string, sizeof(user_type));
        user_type[sizeof(user_type) - 1] = '\0';
        comm_status = send_string(socket, user_type);
        if (comm_status != COMM_SUCCESS) {
            clear_user_list(users);
            return handle_communication_signal(comm_status, __func__);
        }
    }

    // Pulizia della lista utenti allocata localmente
    clear_user_list(users);

    return outcome;
}

OperationOutcome handle_registration(int socket, User** new_user) {
    CommunicationStatus comm_status;
    OperationOutcome outcome = POSITIVE;
    char username[USERNAME_BUFFER_SIZE];
    char password[PASSWORD_BUFFER_SIZE];
    char email[EMAIL_BUFFER_SIZE];
    UserList* users = NULL;

    // Handshake di disponibilità dati
    DataAvailability availability;
    comm_status = receive_availability(socket, &availability);
    if (comm_status != COMM_SUCCESS) {
      return handle_communication_signal(comm_status, __func__);
    }

    // check immediato
    if (availability != DATA_AVAILABLE) {
      return outcome;
    }

    // Salva i dati ricevuti nei buffer locali
	comm_status = receive_string(socket, username, USERNAME_BUFFER_SIZE);
	if (comm_status != COMM_SUCCESS) {
		return handle_communication_signal(comm_status, __func__);
	}
	comm_status = receive_string(socket, password, PASSWORD_BUFFER_SIZE);
	if (comm_status != COMM_SUCCESS) {
		return handle_communication_signal(comm_status, __func__);
	}
	comm_status = receive_string(socket, email, EMAIL_BUFFER_SIZE);
	if (comm_status != COMM_SUCCESS) {
		return handle_communication_signal(comm_status, __func__);
	}

    // Caricamento della lista utenti in modalità scrittura
    users = load_user_list("r+");
    if (users == NULL) {
        outcome = ERROR;
        goto send_result;
    }

    // Controlla se l'username e/o la mail esistono già
    if ((get_user_by_username(users, username) != NULL) ||
        (get_user_by_email(users, email) != NULL)) {
        outcome = NEGATIVE;
        goto send_result;
    }

    // Registrazione del nuovo utente
    *new_user = create_new_user(username, password, email, CLIENT);
    if (!(*new_user)) {
        outcome = ERROR;
        goto send_result;
    }

	// Aggiunta dell'utente alla lista
	if(add_user_to_list(users,*new_user ) == -1) {
		outcome = ERROR;
		goto send_result;
	}

    // Salvataggio della lista aggiornata nel file json
    if (save_user_list(users) == -1) {
        outcome = ERROR;
		goto send_result;
    }

send_result:
    // Invio dell'esito finale al client
    comm_status = send_outcome(socket, outcome);

    // Se la comunicazione fallisce, puliamo la memoria prima di gestire l'errore
    if (comm_status != COMM_SUCCESS) {
        clear_user_list(users);
        return handle_communication_signal(comm_status, __func__);
    }

    // Pulizia finale delle risorse
    clear_user_list(users);
    return outcome;
}
