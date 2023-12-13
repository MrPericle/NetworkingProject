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
#define PORT 54321
#define SOCKET int

void request_exam_availability(char course[]) {
    
    // Richiedi al server se ci sono esami disponibili per un corso
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    int byteRead = 0;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(PORT);
    int tentativo = 0;
    while (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        printf("Connection failed, retry...\n");
        if(++tentativo == 3){
            tentativo = 0;
            printf("waiting random time...\n");
            sleep(rand()%5);
        }
    }
    printf("Connection with server done\n");
    char request_type[] = "REQUEST_EXAM_DATES";
    FullWrite(client_socket, request_type, sizeof(request_type));
    printf("Sending %s\n", request_type);
    sleep(3);
    FullWrite(client_socket, course, strlen(course));
    //printf("written %d char\n",nfwrite);
    printf("sent request for %s\n",course);


    // Ricevi e stampa le date degli esami disponibili per il corso specificato
    char exam_dates[500];
    byteRead = read(client_socket, exam_dates, sizeof(exam_dates));
    if(byteRead == 0)
        printf("Nessuna data disponibile");
    else if(byteRead > 0){
        exam_dates[byteRead] = '\0';
        printf("Date degli esami disponibili per il corso %s:\n%s", course, exam_dates);

    }
    else perror("read error");

    close(client_socket);
}

void reserve_exam(const char* course) {
    // Richiedi la prenotazione di un esame al server
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    int byteRead = 0;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Errore nella connessione al server");
        exit(EXIT_FAILURE);
    }

    char request_type[] = "RESERVE_EXAM";
    FullWrite(client_socket, request_type, sizeof(request_type));
    sleep(3);
    FullWrite(client_socket, course, sizeof(course));

    // Ricevi e stampa la conferma della prenotazione
    char confirmation[50];
    byteRead =  read(client_socket, confirmation, sizeof(confirmation));
    confirmation[byteRead] = '\0';
    printf("%s", confirmation);

    close(client_socket);
}

int main() {
    // Esempio di utilizzo
    // TODO: menù scelta operazione
    char course_to_check[] = "Matematica\0";

    request_exam_availability(course_to_check);

    char course_to_reserve[] = "Fisica";
    reserve_exam(course_to_reserve);

    return 0;
}
