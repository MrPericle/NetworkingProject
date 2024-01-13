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
#include <sys/mman.h>

#define PORT 12345
#define MAX_CONNECTIONS 5
#define MAX_DATE 100
#define MAX_EXAM 30
#define SOCKET int

typedef struct {
    char course[50];
    char exam_date[20];
} Exam;

typedef struct Reservation {
    char name[50];
    int reservation_num;
} Reservation;

Reservation *reservation_data;

Exam exams[MAX_DATE];
int num_exams = 0;
int reservation_index = 0;

void handle_exam_add(SOCKET);

void load_exams_from_file() {
    FILE *file = fopen("exams.txt", "r");
    if (file == NULL) {
        perror("Error opening exams file");
        exit(EXIT_FAILURE);
    }
    num_exams = 0;
    while (fscanf(file, "%49s %19s", exams[num_exams].course, exams[num_exams].exam_date) == 2) {
        num_exams++;
        if (num_exams >= MAX_DATE) {
            break;
        }
    }

    fclose(file);
}

void load_reservation_from_file() {
    FILE *file = fopen("reservations.txt", "r");
    if (file == NULL) {
        perror("Error opening reservations file");
        exit(EXIT_FAILURE);
    }
    reservation_index = 0;
    char arr[50];
    while (fscanf(file, "%49s %19s", reservation_data[reservation_index].name, arr) == 2) {
        reservation_data[reservation_index].reservation_num = atoi(arr);
        reservation_index++;
        if (reservation_index >= MAX_EXAM) {
            break;
        }
    }

    fclose(file);
}

void add_exam(SOCKET client_socket, const char* course, const char* date) {
    if (num_exams < MAX_DATE) {
        strcpy(exams[num_exams].course, course);
        strcpy(exams[num_exams].exam_date, date);
    }

    handle_exam_add(client_socket);
}

void handle_exam_request(SOCKET client_socket, char* course) {
    // Send available exam dates to the client
    load_exams_from_file();
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

void handle_exam_reservation(SOCKET client_socket, const char* course, const char* date) {
    // Handle exam reservation (simulated)
    // Here you could implement actual reservation logic
    // For example, save the reservation to a file
    
    int found = 0;
    char buffer[100];
    load_reservation_from_file();

    for(int i = 0; i < MAX_DATE && found == 0; i++){
        if(!strcmp(course, exams[i].course)){
            if(!strcmp(date, exams[i].exam_date)){
                found = 1;
            }
        }
    }
    if(found == 1){

        FILE *reservation_file = fopen("reservations.txt", "w");
        if (reservation_file == NULL) {
            perror("\nError opening reservations file");
            return;
        }
        fclose(reservation_file);

        reservation_file = fopen("reservations.txt", "a");

        int i;    
        for(i = 0; i < reservation_index; i++) 
            if(!strcmp(course, reservation_data[i].name))
                break;

        if(i >= reservation_index){
            strcpy(reservation_data[i].name, course);
            reservation_data[i].reservation_num = 1;
            reservation_index++;
        }
        else
            reservation_data[i].reservation_num++; 


        for(int j = 0; j < reservation_index; j++){
            printf("Index %d\n", reservation_index);
            printf("Name %s, Num %d\n", reservation_data[j].name, reservation_data[j].reservation_num);
            fprintf(reservation_file, "%s %d\n", reservation_data[j].name, reservation_data[j].reservation_num);
        }

        
        fclose(reservation_file);

        snprintf(buffer,sizeof(buffer),"reservation number %d \n\n", reservation_data[i].reservation_num);
        printf("Sending %lu",strlen(buffer));
        FullWrite(client_socket,buffer, strlen(buffer));
    }
    else{

        snprintf(buffer, sizeof(buffer),"\nNot found exam  %s for date %s\n", course, date);
        printf("Sending %lu",strlen(buffer));
        FullWrite(client_socket,buffer, strlen(buffer));
    }
}

void handle_exam_add(SOCKET client_socket){

    // Add exam to the file
    FILE *file = fopen("exams.txt", "a");
    if (file == NULL) {
        perror("Error opening exams file");
        exit(EXIT_FAILURE);
    }

    // Check if there is enough space to add an exam
    if (num_exams >= MAX_DATE) {
        fprintf(stderr, "\nError: Maximum number of exams reached\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Write the new exam to the file
    fprintf(file, "%s %s\n", exams[num_exams].course, exams[num_exams].exam_date);

    // Increment the number of exams and close the file
    load_exams_from_file();

    write(client_socket, "\nExam added successfully!\0", 30);
    
    fclose(file);
}

int main(){

    printf("\n The University Server is getting ready to start... \n");
    // Create shared memory for the array of integers (val[10])
    reservation_data = mmap(NULL, sizeof(Reservation) * MAX_EXAM, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    load_exams_from_file();
    load_reservation_from_file();
    printf("\nLoaded exams from file\n");
 
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Initialize the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket to address and port");
        exit(EXIT_FAILURE);
    }

    // Put the server in listening mode
    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("Error initializing socket listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Fork to handle the connection
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error creating child process");
            close(client_socket);
            continue;
        } else if (pid > 0) {
            wait(NULL);
            // Parent process - close the client socket and continue accepting new connections
            close(client_socket);
        } else {
            close(server_socket); // Child process does not need the server socket
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
            if(!bytes_read){
                printf("Client closed connection\n");
                close(client_socket);
            }
            else{
                request_type[bytes_read] = '\0';
                printf("\nReading request type %s\n", request_type);
            } 

            // Read the course
            bytes_read = read(client_socket, course, sizeof(course));
            if(!bytes_read){
                printf("Client closed connection\n");
                close(client_socket);
            }
            else {
                course[bytes_read] = '\0';
                printf("\nReading course: %s\n", course);
            };

            if (strcmp(request_type, "REQUEST_EXAM_DATES") == 0) {
                // Handle the request for exam dates
                handle_exam_request(client_socket,course);
            } else if (strcmp(request_type, "RESERVE_EXAM") == 0) {
                bytes_read2 = read(client_socket, date, sizeof(date));
                date[bytes_read2] = '\0';
                printf("\nDate: %s \n", date);
                // Handle the exam reservation
                handle_exam_reservation(client_socket,course, date);
            } else if(strcmp(request_type, "ADD_EXAM") == 0){
                bytes_read2 = read(client_socket, date, sizeof(date));
                date[bytes_read2] = '\0';
                printf("\nDate: %s \n", date);
                add_exam(client_socket, course, date);
            }
            else{
                perror("\nUnknown request");
                exit(EXIT_FAILURE);
            }

            // Close the connection with the current client
            close(client_socket);
            exit(EXIT_SUCCESS); // Terminate the child process
        }
    }

    // Close the server socket
    close(server_socket);
    
    munmap(reservation_data, sizeof(Reservation) * MAX_EXAM); // Deallocation of shared memory

    return 0;
}
