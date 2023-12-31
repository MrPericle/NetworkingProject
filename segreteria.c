#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "wrapper.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define LOCAL_PORT 54321
#define SOCKET int

void request_exam_dates(SOCKET student_socket, const char *course)
{
    // Richiedi al server universitario le date degli esami
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Errore nella connessione al server universitario");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "REQUEST_EXAM_DATES";
    FullWrite(client_socket, request_type, sizeof(request_type));
    printf("Request forwarded to server");
    sleep(3);
    FullWrite(client_socket, course, strlen(course));
    printf("Course forwarded to server\n");

    // Ricevi le date degli esami disponibili dal server universitario
    char exam_dates[500];
    int nread = read(client_socket, exam_dates, sizeof(exam_dates));
    exam_dates[nread] = '\0';

    // Inoltra le date al client studente
    FullWrite(student_socket, exam_dates, strlen(exam_dates));

    close(client_socket);
}

void forward_exam_reservation(int student_socket, const char *course)
{

    // Inoltra la richiesta di prenotazione al server universitario
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Errore nella connessione al server universitario");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "RESERVE_EXAM";
    printf("%s", course);
    FullWrite(client_socket, request_type, sizeof(request_type));
    sleep(3);
    FullWrite(client_socket, course, strlen(course));

    // Ricevi e inoltra la conferma della prenotazione al client studente
    char confirmation[100];
    int bRead = read(client_socket, confirmation, sizeof(confirmation));
    confirmation[bRead] = '\0';
    FullWrite(student_socket, confirmation, strlen(confirmation));

    close(client_socket);
}

void add_exam(const char *course, const char *date)
{

    // Inoltra la richiesta di prenotazione al server universitario
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Errore nella connessione al server universitario");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "ADD_EXAM";
    int byteRead = 0;

    FullWrite(client_socket, request_type, sizeof(request_type));
    sleep(3);

    FullWrite(client_socket, course, sizeof(course));
    sleep(3);

    FullWrite(client_socket, date, sizeof(date));

    // Ricevi conferma dell'aggiunta con successo
    char confirmation[100];
    byteRead = read(client_socket, confirmation, sizeof(confirmation));

    if (byteRead == 0)
    {
        printf("Nessun esame inserito\n");
    }
    else
    {
        confirmation[byteRead] = '\0';
        printf("Inserito Esame %s: il %s\n", course, date);
    }

    close(client_socket);
}

int main()
{
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Inizializza il socket del server e gestisci eventuali errori
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Errore nella creazione del socket del server");
        exit(EXIT_FAILURE);
    }

    // Inizializza l'indirizzo del server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(LOCAL_PORT);

    // Lega il socket del server all'indirizzo del server
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Errore nella bind del socket del server");
        exit(EXIT_FAILURE);
    }

    // Inizia ad ascoltare le connessioni in entrata
    if (listen(server_socket, 1) == -1)
    {
        perror("Errore nella listen del socket del server");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Accetta una connessione dallo studente
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1)
        {
            perror("Errore nell'accettazione della connessione dello studente");
            continue;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Errore nella creazione del processo figlio");
        }
        else if (pid == 0)
        {
            close(server_socket); // Chiudi il socket del server nel processo figlio
            printf("Child handling request\n");

            // Leggi il tipo di richiesta dallo studente
            char request_type[30];
            char course[100];
            ssize_t bytes_read;

            // Leggi il tipo di richiesta
            bytes_read = read(client_socket, request_type, sizeof(request_type));
            if (!bytes_read)
            {
                printf("client closed connection\n");
                close(client_socket);
            }
            else
                printf("\nReading request type %s\n", request_type);

            // Leggi il corso
            bytes_read = read(client_socket, course, sizeof(course));
            if (!bytes_read)
            {
                printf("client closed connection\n");
                close(client_socket);
            }
            else
            {
                course[bytes_read] = '\0';
                printf("\nReading course: %s\n", course);
            };

            if (strcmp(request_type, "REQUEST_EXAM_DATES") == 0)
            {

                // Gestisci la richiesta di date degli esami
                printf("request 1 sent\n");
                printf("strlen in server %lu\n", strlen(course));
                request_exam_dates(client_socket, course);
            }
            else if (strcmp(request_type, "RESERVE_EXAM") == 0)
            {

                // Ricevi il corso per la prenotazione e inoltralo al server universitario
                printf("request 2 sent\n");
                forward_exam_reservation(client_socket, course);
            }
            else
            {

                perror("Unrecognized request");
                exit(EXIT_FAILURE);
            }

            // Chiudi la connessione con lo studente corrente
            //close(client_socket);
            exit(EXIT_SUCCESS);
        }
        else
        {
            int pid_client = fork();
            if (pid_client == 0)
            {
                close(server_socket);
                char choice;
                char course[100];
                char date[100];

                while (1)
                {
                    printf("\nVuoi Inserire un nuovo esame?\n [Y/n]: ");
                    scanf("%c", &choice);

                    if (choice == 'y')
                    {
                        printf("\nInserisci l'esame: ");
                        scanf("%s", course);

                        printf("\nInserisci la data nel formato DD/MM/YY: ");
                        scanf("%s", date);

                        add_exam(course, date);
                    }
                    else
                    {
                        sleep(20);
                    }
                }
                close(client_socket);
            }
            else
                close(client_socket);
        }
    }

    // Chiudi il socket del server (questo punto potrebbe non essere mai raggiunto)
    close(server_socket);

    return 0;
}
