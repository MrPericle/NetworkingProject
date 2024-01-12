#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "wrapper.h"

#define PORT 12345
#define MAX_CONNECTIONS 5
#define MAX_EXAMS 100
#define SOCKET int

typedef struct {
    char course[50];
    char exam_date[20];
} Exam;

Exam exams[MAX_EXAMS];
int num_exams = 0;
int num_prenot = 1;
int *num_prenot_pt = &num_prenot;

void handler(int sigum){
    if(sigum == SIGUSR1){
        (*num_prenot_pt)++;
    }
}

void handle_exam_add(SOCKET);

void load_exams_from_file() {
    FILE *file = fopen("exams.txt", "r");
    if (file == NULL) {
        perror("\nError opening the exams file.");
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%49s %19s", exams[num_exams].course, exams[num_exams].exam_date) == 2) {
        num_exams++;
        if (num_exams >= MAX_EXAMS) {
            break;
        }
    }

    fclose(file);
}

void add_exam(SOCKET client_socket, const char* course, const char* date) {
    if (num_exams < MAX_EXAMS) {
        strcpy(exams[num_exams].course, course);
        strcpy(exams[num_exams].exam_date, date);
    }

    handle_exam_add(client_socket);
}

void handle_exam_request(SOCKET client_socket, const char* course) {
    // Cerca le date degli esami disponibili 
    char exam_dates[500] = "";
    int exam_found = 0;

    for (int i = 0; i < num_exams; ++i) {
        if (strcmp(exams[i].course, course) == 0) {
            snprintf(exam_dates + strlen(exam_dates), sizeof(exam_dates) - strlen(exam_dates), "%s: %s\n", exams[i].course, exams[i].exam_date);
            exam_found = 1;
            // Interrompi la ricerca una volta trovata la corrispondenza
            break;
        }
    }

    if (exam_found) {
        FullWrite(client_socket, exam_dates, strlen(exam_dates));
    } else {
        FullWrite(client_socket, "EXAM_NOT_FOUND", sizeof("EXAM_NOT_FOUND"));
    }
}


void handle_exam_reservation(SOCKET client_socket, const char* course) {
    // Gestisci la prenotazione dell'esame (simulato)
    // Qui potresti implementare la logica di gestione delle prenotazioni effettive
    // Ad esempio, salva la prenotazione su un file
    char buffer[100];
    FILE *reservation_file = fopen("reservations.txt", "a");
    if (reservation_file == NULL) {
        perror("\nError opening the reservations file.");
        return;
    }
    kill(getppid(),SIGUSR1);

    fprintf(reservation_file, "\nReservation %d has booked the exam for %s\n", *num_prenot_pt, course);
    fclose(reservation_file);

    snprintf(buffer,sizeof(buffer),"\nReservation Number %d", *num_prenot_pt);
    printf("Sending %lu",strlen(buffer));
    FullWrite(client_socket,buffer, strlen(buffer));
}

void handle_exam_add(SOCKET client_socket){

    //aggiungi esame al file
    FILE *file = fopen("exams.txt", "a");
    if (file == NULL) {
        perror("Error opening the exams file.");
        exit(EXIT_FAILURE);
    }

    // Verifica se c'è spazio sufficiente per aggiungere un esame
    if (num_exams >= MAX_EXAMS) {
        fprintf(stderr, "\nError: The maximum number of exams has been reached.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Scrivi il nuovo esame nel file
    fprintf(file, "%s %s\n", exams[num_exams].course, exams[num_exams].exam_date);

    // Incrementa il numero di esami e chiudi il file
    num_exams++;

    FullWrite(client_socket, "\n- Exam added successfully!\0", 30);

    fclose(file);

}

int main() {
    signal(SIGUSR1, handler);
    load_exams_from_file();
    printf("Loaded exams from file...\n");

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Initialize the socket
    server_socket = Socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error in creating the socket.");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (Bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("\nError in binding the socket to the address and port.");
        exit(EXIT_FAILURE);
    }

    // Put the server in listening mode
    if (Listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("\nError in initializing the socket listening.");
        exit(EXIT_FAILURE);
    }

    printf("\nServer listening on port %d...\n", PORT);

    while (1) {
        // Accept a connection
        client_socket = Accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error in accepting the connection");
            continue;
        }

        // Fork to handle the connection
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error in creating the child process");
            close(client_socket);
            continue;
        } else if (pid > 0) {
            wait(NULL);
            // Parent process - close the client socket and continue accepting new connections
            close(client_socket);
        } else {
            close(server_socket); // The child process doesn't need the server socket
            printf("Child handling request\n");
            // Child process - handle the connection

            // Receive the request type from the client
            char request_type[30];
            char course[100];
            char date[100];
            ssize_t bytes_read;
            ssize_t bytes_read2;

            // Read the request type
            bytes_read = read(client_socket, request_type, sizeof(request_type));
            if (!bytes_read) {
                printf("Client closed connection\n");
                close(client_socket);
            } else {
                request_type[bytes_read] = '\0';
                printf("\nReading request type %s\n", request_type);
            }

            // Read the course
            bytes_read = read(client_socket, course, sizeof(course));
            if (!bytes_read) {
                printf("Client closed connection\n");
                close(client_socket);
            } else {
                course[bytes_read] = '\0';
                printf("\nReading course: %s\n", course);
            };

            if (strcmp(request_type, "REQUEST_EXAM_DATES") == 0) {

                // Handle the request for exam dates
                handle_exam_request(client_socket, course);

            } else if (strcmp(request_type, "RESERVE_EXAM") == 0) {

                // Handle the exam reservation
                handle_exam_reservation(client_socket, course);

            } else if (strcmp(request_type, "ADD_EXAM") == 0) {

                bytes_read2 = read(client_socket, date, sizeof(date));
                printf("\nData: %s \n", date);
                add_exam(client_socket, course, date);

            } else {
                perror("\nRequest unknown");
                exit(EXIT_FAILURE);
            }

            // Close the connection with the current client
            close(client_socket);
            exit(EXIT_SUCCESS); // Terminate the child process
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
