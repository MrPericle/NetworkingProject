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
#define MAX_RETRY 3


void establish_connection(SOCKET *client_socket) {
    int attempt = 0;

    while (attempt < MAX_RETRY) {
        *client_socket = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_address = {0};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
        server_address.sin_port = htons(PORT);

        if (Connect(*client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == 0)            
            break;
        else {
            close(*client_socket);
            attempt++;

            if (attempt == MAX_RETRY) {
                int random_time = rand() % 5;
                printf("Waiting for %d seconds...\n", random_time);
                sleep(random_time);

                // Reset attempts
                attempt = 0;
            }
        }
    }
}

void request_exam_availability(char course[]) {
    SOCKET client_socket;
    int byte_read = 0;

    establish_connection(&client_socket);

    char request_type[] = "REQUEST_EXAM_DATES";
    FullWrite(client_socket, request_type, sizeof(request_type));
    printf("Sending request: %s\n", request_type);
    sleep(3);
    FullWrite(client_socket, course, strlen(course));

    char exam_dates[500];
    byte_read = read(client_socket, exam_dates, sizeof(exam_dates));
    exam_dates[byte_read] = '\0';

    if (strcmp(exam_dates, "EXAM_NOT_FOUND") == 0) {
        printf("No available exam dates for %s\n", course);
    } else {
        printf("Available exam dates for %s:\n%s", course, exam_dates);
    }

    close(client_socket);
}


void reserve_exam(const char* course) {
    SOCKET client_socket;
    int byte_read = 0;

    establish_connection(&client_socket);

    char request_type[] = "RESERVE_EXAM";
    FullWrite(client_socket, request_type, sizeof(request_type));
    sleep(3);
    FullWrite(client_socket, course, strlen(course));

    char confirmation[100];
    byte_read =  read(client_socket, confirmation, sizeof(confirmation));
    confirmation[byte_read] = '\0';
    printf("Reservation confirmation: %s", confirmation);

    close(client_socket);
}

int main() {
    int choice;
    
    while(1){
        printf("\n\n1. Request Exam Availability: \n");
        printf("2. Reserve Exam\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                char course_to_check[50];
                printf("\nEnter exam name: ");
                scanf("%s",course_to_check);
                request_exam_availability(course_to_check);
                break;
            }
            case 2: {
                char course_to_reserve[50];
                printf("\nEnter exam name: ");
                scanf("%s",course_to_reserve);
                reserve_exam(course_to_reserve);
                break;
            }
            default:
                printf("\n** Sorry, this choiche is invalid **\n");
                break;
        }
    }

    return 0;
}

