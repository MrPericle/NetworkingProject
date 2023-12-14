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
int num_prenot = 0;
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
        perror("Errore nell'apertura del file degli esami");
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

void handle_exam_request(SOCKET client_socket,char* course) {
    // Invia al client le date degli esami disponibili 
    char exam_dates[500] = "";
    for (int i = 0; i < num_exams; ++i) {
        if(strcmp(exams[i].course,course) == 0){
            strcat(exam_dates, exams[i].course);
            strcat(exam_dates, ": ");
            strcat(exam_dates, exams[i].exam_date);
            strcat(exam_dates, "\n");
        }
    }
    write(client_socket, exam_dates, strlen(exam_dates));
}

void handle_exam_reservation(SOCKET client_socket, const char* course) {
    // Gestisci la prenotazione dell'esame (simulato)
    // Qui potresti implementare la logica di gestione delle prenotazioni effettive
    // Ad esempio, salva la prenotazione su un file
    char buffer[100];
    FILE *reservation_file = fopen("reservations.txt", "a");
    if (reservation_file == NULL) {
        perror("Errore nell'apertura del file delle prenotazioni");
        return;
    }
    kill(getppid(),SIGUSR1);

    fprintf(reservation_file, "Prenotato %d ha prenotato l'esame di %s\n", *num_prenot_pt, course);
    fclose(reservation_file);

    snprintf(buffer,sizeof(buffer),"Prenotazione effettuata con successo con numero prenotazione %d", *num_prenot_pt);
    printf("Sending %lu",strlen(buffer));
    FullWrite(client_socket,buffer, strlen(buffer));
}


void handle_exam_add(SOCKET client_socket){

    //aggiungi esame al file
    FILE *file = fopen("exams.txt", "a");
    if (file == NULL) {
        perror("Errore nell'apertura del file degli esami");
        exit(EXIT_FAILURE);
    }

    // Verifica se c'è spazio sufficiente per aggiungere un esame
    if (num_exams >= MAX_EXAMS) {
        fprintf(stderr, "Errore: Il numero massimo di esami è stato raggiunto\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Scrivi il nuovo esame nel file
    fprintf(file, "%s %s\n", exams[num_exams].course, exams[num_exams].exam_date);

    // Incrementa il numero di esami e chiudi il file
    num_exams++;

    write(client_socket, "Esame aggiunto con successo!\0", 30);

    fclose(file);

}

int main(){
    signal(SIGUSR1,handler);
    load_exams_from_file();
    printf("loaded exams from file\n");
 
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
    server_address.sin_port = htons(PORT);

    // Associa il socket all'indirizzo e alla porta
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Errore nell'associazione del socket all'indirizzo e alla porta");
        exit(EXIT_FAILURE);
    }

    // Mette il server in ascolto
    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("Errore nell'inizializzazione dell'ascolto del socket");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    while (1) {
        // Accetta una connessione
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Errore nell'accettazione della connessione");
            continue;
        }

        // Fork per gestire la connessione
        pid_t pid = fork();
        if (pid == -1) {
            perror("Errore nella creazione del processo figlio");
            close(client_socket);
            continue;
        } else if (pid > 0) {
            wait(NULL);
            // Processo padre - chiude il socket del client e continua ad accettare nuove connessioni
            close(client_socket);
        } else {
            close(server_socket); // Il processo figlio non ha bisogno del socket del server
            printf("child handling request\n");
            // Processo figlio - gestisce la connessione

            // Ricevi il tipo di richiesta dal client
            char request_type[30];
            char course[100];
            char date[100];
            ssize_t bytes_read;
            ssize_t bytes_read2;

            // Leggi il tipo di richiesta
            bytes_read = read(client_socket, request_type, sizeof(request_type));
            if(!bytes_read){
                printf("client closed connection\n");
                close(client_socket);
            }
            else{
                request_type[bytes_read] = '\0';
                printf("Reading request type %s\n", request_type);
            } 

            // Leggi il corso
            bytes_read = read(client_socket, course, sizeof(course));
            if(!bytes_read){
                printf("client closed connection\n");
                close(client_socket);
            }
            else {
                course[bytes_read] = '\0';
                printf("Reading course: %s\n", course);
                
            };
            if (strcmp(request_type, "REQUEST_EXAM_DATES") == 0) {

                // Gestisci la richiesta di date degli esami
                handle_exam_request(client_socket,course);

            } else if (strcmp(request_type, "RESERVE_EXAM") == 0) {

                // Gestisci la prenotazione dell'esame
                handle_exam_reservation(client_socket,course);

            }else if(strcmp(request_type, "ADD_EXAM") == 0){

                bytes_read2 = read(client_socket, date, sizeof(date));
                printf("\ndata: %s \n", date);
                add_exam(client_socket, course, date);

            }
            else{
                perror("Request unknown");
                exit(EXIT_FAILURE);
            }

            // Chiudi la connessione con il client corrente
            close(client_socket);
            exit(EXIT_SUCCESS); // Termina il processo figlio
        }
    }

    // Chiudi il socket del server
    close(server_socket);

    return 0;
}