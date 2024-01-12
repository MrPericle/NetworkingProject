#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include "wrapper.h"

// Funzione per creare un socket.
int Socket(int family, int type, int protocol)
{
    int n;
    if ((n = socket(family, type, protocol)) < 0) {
        perror("socket"); // Stampa un messaggio di errore e la descrizione dell'errore corrispondente.
        exit(1); // Uscita forzata del programma in caso di errore.
    }
    return (n); // Restituisce il descrittore del socket creato.
}

// Funzione per connettersi a un server remoto.
int Connect(int socket, const struct sockaddr *servaddr, socklen_t address_len)
{
    if (connect(socket, servaddr, address_len) < 0) {
        fprintf(stderr, "Connection failed, retrying...\n"); // Stampa un messaggio di errore su stderr.
        return -1; // Indica un errore di connessione.
    } else {
        printf("Connection established\n");
        return 0; // Indica che la connessione è stata stabilita con successo.
    }
}

// Funzione per associare un indirizzo locale a un socket.
void Bind(int socket, const struct sockaddr *servaddr, socklen_t address_len)
{
    if (bind(socket, servaddr, address_len) < 0) {
        perror("Errore nella bind del socket del server"); // Stampa un messaggio di errore e la descrizione dell'errore corrispondente.
        exit(1); // Uscita forzata del programma in caso di errore.
    }
}

// Funzione per mettere in ascolto un socket.
void Listen(int socket, int backlog)
{
    if (listen(socket, backlog) < 0) {
        perror("Errore nella listen del socket del server"); // Stampa un messaggio di errore e la descrizione dell'errore corrispondente.
        exit(1); // Uscita forzata del programma in caso di errore.
    }
}

// Funzione per accettare una connessione in entrata.
int Accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len)
{
    int connfd;
    if ((connfd = accept(socket, address, address_len)) < 0) {
        perror("accept"); // Stampa un messaggio di errore e la descrizione dell'errore corrispondente.
        exit(1); // Uscita forzata del programma in caso di errore.
    }
    return connfd; // Restituisce il descrittore del socket accettato.
}

// Funzione per leggere dati da un file descriptor in modo completo.
ssize_t FullRead(int fd, void *buf, size_t count)
{
    size_t nleft;
    ssize_t nread;
    nleft = count;

    while (nleft > 0) {
        if ((nread = read(fd, buf, nleft)) < 0) {
            if (errno == EINTR)
                continue; // Se l'errore è causato da un'interruzione, continua la lettura.
            else
                exit(EXIT_FAILURE); // Uscita forzata del programma in caso di errore diverso da un'interruzione.
        } else if (nread == 0) //Ottenere 0 bytes significa che il socket e' vuoto ed e' stato chiuso
            break; // Se la lettura non restituisce dati (EOF), esci dal ciclo.
        nleft -= nread;
        buf += nread; // Sposta il puntatore nel buffer per leggere i dati rimanenti.
    }
    buf = 0; // Non è chiaro cosa si intenda fare qui; sembra inutile.

    return (nleft); // Restituisce il numero di byte che mancano da leggere.
}

// Funzione per scrivere dati su un file descriptor in modo completo.
ssize_t FullWrite(int fd, const void *buf, size_t count)
{
    size_t nleft;
    ssize_t nwritten;
    nleft = count;

    while (nleft > 0) {
        if ((nwritten = write(fd, buf, nleft)) < 0) {
            if (errno == EINTR)
                continue; // Se l'errore è causato da un'interruzione, continua la scrittura.
            else
                exit(EXIT_FAILURE); // Uscita forzata del programma in caso di errore diverso da un'interruzione.
        } else {
            nleft -= nwritten;
            buf += nwritten; // Sposta il puntatore nel buffer per scrivere i dati rimanenti.
        }
    }
    buf = 0;
    return (nleft); // Restituisce il numero di byte che mancano da scrivere.
}