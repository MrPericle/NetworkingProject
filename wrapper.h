#ifndef WRAPPER_H
#define WRAPPER_H

// Prototipo della funzione wrapper per creare un socket.
int Socket(int family, int type, int protocol);

// Prototipo della funzione wrapper per connettersi a un server remoto.
void Connect(int socket, const struct sockaddr *address, socklen_t address_len);

// Prototipo della funzione wrapper per associare un indirizzo locale a un socket.
void Bind(int socket, const struct sockaddr *address, socklen_t address_len);

// Prototipo della funzione wrapper per mettere in ascolto un socket.
void Listen(int socket, int backlog);

// Prototipo della funzione wrapper per accettare una connessione in entrata.
int Accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);

// Prototipo della funzione per leggere dati da un file descriptor in modo completo.
ssize_t FullRead(int fd, void *buf, size_t count);

// Prototipo della funzione per scrivere dati su un file descriptor in modo completo.
//FullWrite scrive esattamente count byte s iterando opportunamente le scritture
ssize_t FullWrite(int fd, const void *buf, size_t count);

#endif // WRAPPER_H
