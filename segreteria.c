#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define LOCAL_PORT 54321
#define SOCKET int

void request_exam_dates(SOCKET student_socket) {
    
    // Richiedi al server universitario le date degli esami
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Errore nella connessione al server universitario");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "REQUEST_EXAM_DATES";
    write(client_socket, request_type, sizeof(request_type));

    // Ricevi le date degli esami disponibili dal server universitario
    char exam_dates[500];
    read(client_socket, exam_dates, sizeof(exam_dates));

    // Inoltra le date al client studente
    write(student_socket, exam_dates, strlen(exam_dates));

    close(client_socket);
}

void forward_exam_reservation(int student_socket, const char* course) {
    // Inoltra la richiesta di prenotazione al server universitario
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Errore nella connessione al server universitario");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "RESERVE_EXAM";
    write(client_socket, request_type, sizeof(request_type));
    write(client_socket, course, strlen(course));

    // Ricevi e inoltra la conferma della prenotazione al client studente
    char confirmation[50];
    read(client_socket, confirmation, sizeof(confirmation));
    write(student_socket, confirmation, strlen(confirmation));

    close(client_socket);
}

int main() {
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Inizializza il socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Configura l'indirizzo del server
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(LOCAL_PORT);

    // Associa il socket all'indirizzo e alla porta
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Errore nell'associazione del socket all'indirizzo e alla porta");
        exit(EXIT_FAILURE);
    }

    // Mette il server in ascolto
    if (listen(server_socket, 1) < 0) {
        perror("Errore nell'inizializzazione dell'ascolto del socket");
        exit(EXIT_FAILURE);
    }

    printf("Segreteria in ascolto sulla porta %d...\n", LOCAL_PORT);

    while (1) {
        
        // Accetta una connessione dallo studente
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Errore nell'accettazione della connessione dello studente");
            continue;
        }
        pid_t pid = fork();
         if (pid == -1) {
            perror("Errore nella creazione del processo figlio");
        } else if (pid == 0) {
            close(server_socket);
            // Ricevi il tipo di richiesta dallo studente
            char request_type[20];
            read(client_socket, request_type, sizeof(request_type));

            if (strcmp(request_type, "REQUEST_EXAM_DATES") == 0) {
                // Gestisci la richiesta di date degli esami
                request_exam_dates(client_socket);
            } else if (strcmp(request_type, "RESERVE_EXAM") == 0) {
                // Ricevi il corso per la prenotazione e inoltralo al server universitario
                char course[50];
                read(client_socket, course, sizeof(course));
                forward_exam_reservation(client_socket,course);
            }
        
            // Chiudi la connessione con lo studente corrente
            close(client_socket);
            exit(0);
        }
        else close(client_socket);
    }

    // Chiudi il socket del server
    close(server_socket);

    return 0;
}
