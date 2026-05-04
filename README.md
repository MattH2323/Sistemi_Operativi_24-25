# Ticketing System - Help Desk

Progetto di **Sistemi Operativi 2024/2025** presso l'Università degli Studi di Firenze (Scuola di Scienze Matematiche Fisiche e Naturali).

---

## 👥 Autori
*   **Xiao Kang Luo** - 7137112 (xiao.luo@edu.unifi.it)
*   **Silvia Zhu** - 7055499 (silvia.zhu@edu.unifi.it)

---

## 🏗️ Architettura del Sistema
Il sistema segue un'architettura **Client-Server** basata su socket nel dominio **AF_INET** di tipo **SOCK_STREAM** (protocollo TCP).

### Caratteristiche Tecniche:
*   **Concorrenza**: Il server gestisce più client simultaneamente utilizzando la chiamata di sistema `fork()`. Ogni connessione genera un processo figlio dedicato alla sessione.
*   **Persistenza Dati**: Le informazioni su utenti e ticket sono salvate in formato **JSON** nella cartella `data/`.
*   **Librerie Esterne**: Integrazione della libreria [cJSON](https://github.com/DaveGamble/cJSON) per il parsing dei dati.
*   **Gestione Mutua Esclusione**: Accesso ai file protetto tramite meccanismi di locking (`flock`) con `LOCK_SH` per la lettura e `LOCK_EX` per la scrittura.

*N.B.* Per maggiori dettagli consulare `relazione.pdf`

---

## 📂 Organizzazione del Progetto
```text
ticketing_system/
├── bin/                # File eseguibili (client e server)
├── build/              # File oggetto (.o)
├── data/               # Database in formato JSON (tickets.json, users.json)
├── include/            # File di intestazione (.h) suddivisi per modulo
│   ├── client/, json/, server/, ticket/, user/, utils/
├── src/                # File sorgente (.c) suddivisi per modulo
│   ├── client/, json/, server/, ticket/, user/, utils/
└── ticketing_system.make   # Makefile per la compilazione
