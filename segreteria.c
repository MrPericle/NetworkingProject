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
    // Request exam dates from the university server
    SOCKET client_socket = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (Connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("\n** Error connecting to the university server **\n");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "REQUEST_EXAM_DATES";
    FullWrite(client_socket, request_type, sizeof(request_type));
    printf("Request forwarded to the server");
    sleep(3);
    FullWrite(client_socket, course, strlen(course));
    printf("Course forwarded to the server\n");

    // Receive available exam dates from the university server
    char exam_dates[500];
    int nread = read(client_socket, exam_dates, sizeof(exam_dates));
    exam_dates[nread] = '\0';

    // Forward the dates to the student client
    FullWrite(student_socket, exam_dates, strlen(exam_dates));

    close(client_socket);
}

void forward_exam_reservation(int student_socket, const char *course, const char *date)
{
    // Forward the reservation request to the university server
    SOCKET client_socket = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (Connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error connecting to the university server");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "RESERVE_EXAM";
    printf("%s", course);  // Note: This line prints the course without a newline, consider adding a newline character.
    FullWrite(client_socket, request_type, sizeof(request_type));
    sleep(3);
    FullWrite(client_socket, course, strlen(course));
    sleep(3);
    FullWrite(client_socket, date, strlen(date));

    // Receive and forward the reservation confirmation to the student client
    char confirmation[100];
    int bytes_read = read(client_socket, confirmation, sizeof(confirmation));
    confirmation[bytes_read] = '\0';
    FullWrite(student_socket, confirmation, strlen(confirmation));

    close(client_socket);
}

void add_exam(const char *course, const char *date)
{
    // Forward the exam addition request to the university server
    SOCKET client_socket = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    if (Connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error connecting to the university server");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "ADD_EXAM";
    int bytes_read = 0;

    FullWrite(client_socket, request_type, sizeof(request_type));
    sleep(3);

    FullWrite(client_socket, course, strlen(course));
    sleep(3);

    FullWrite(client_socket, date, strlen(date));

    // Receive confirmation of successful addition
    char confirmation[100];
    bytes_read = read(client_socket, confirmation, sizeof(confirmation));

    if (bytes_read == 0)
    {
        printf("No exam added\n");
    }
    else
    {
        confirmation[bytes_read] = '\0';
        printf("Added Exam %s: on %s\n", course, date);
    }

    close(client_socket);
}

int main()
{   
    char course[100];
    char date[100];
    int choice;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Initialize the server socket and handle potential errors
    server_socket = Socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating the server socket");
        exit(EXIT_FAILURE);
    }

    // Initialize the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(LOCAL_PORT);

    // Bind the server socket to the server address
    if (Bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding the server socket");
        exit(EXIT_FAILURE);
    }

    // Inizia ad ascoltare le connessioni in entrata
    if (listen(server_socket, 1) == -1)
    {
        perror("Error in listening on the server socket");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Accept a connection from the student
        client_socket = Accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1)
        {
            perror("Error accepting the student's connection");
            continue;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Error creating a child process");
        }
        else if (pid == 0)
        {
            close(server_socket); // Close the server socket in the child process
            printf("Child handling request\n");

            // Read the type of request from the student
            char request_type[30];
            char course[100];
            char date[50];
            ssize_t bytes_read;

            // Read the type of request
            bytes_read = read(client_socket, request_type, sizeof(request_type));
            if (!bytes_read)
            {
                printf("Client closed connection\n");
                close(client_socket);
            }
            else
                printf("\nReading request type %s\n", request_type);

            // Read the course
            bytes_read = read(client_socket, course, sizeof(course));
            if (!bytes_read)
            {
                printf("Client closed connection\n");
                close(client_socket);
            }
            else
            {
                course[bytes_read] = '\0';
                printf("\nReading course: %s\n", course);
            }

            if (strcmp(request_type, "REQUEST_EXAM_DATES") == 0)
            {

                // Handle the request for exam dates
                printf("Request 1 sent\n");
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

    // Close the server socket (this point may never be reached)
    close(server_socket);

    return 0;
}
