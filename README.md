# Networking Project
The project aims to develop a parallel client/server application to manage university exams. The outline involves the creation of three groups of students, each responsible for a specific part of the application.

# User Manual for Compilation and Execution

## Clone the Repository

To obtain the project's source code, you need to clone the repository from GitHub. Use the following terminal command:

```
 git clone https://github.com/MrPericle/NetworkingProject.git
```
Next, navigate to the newly cloned directory using the command:
```
 cd NetworkingProject
```

## Compilation
The project consists of three distinct executable files: **serverUniversitario**, **segreteria**, and **client**. Each file must be compiled separately, as indicated below:

- **Server Universitario**:
```
 gcc serverUniversitario.c wrapper.c -o serverUniversitario
```
- **Segreteria**:
```
 gcc segreteria.c wrapper.c -o segreteria
```
- **Studente**:
```
 gcc studente.c wrapper.c -o client

```

## Execution
After compilation, run the executables in the following order:

- **Server Universitario**:
```
 ./serverUniversitario
```
- **Segreteria**:
```
 ./segreteria
```
- **Studente**:
```
 ./client
```

## Features
Once the components are started, the university server will remain listening. The main functionalities of the segreteria and student are as follows:

### Segreteria:
1. Inserts exams into the university server.
2. Forwards student reservation requests to the university server.
3. Provides the student with available exam dates for the selected course.

### Studente:
1. Asks the segreteria if there are available exams for a specific course.
2. Sends a reservation request for an exam to the segreteria.

## Additional Features
- The university server, for each reservation request, sends the segreteria the sequentially assigned reservation number for the student, and the segreteria forwards it to the student.

- If the segreteria does not respond to the student's request, the student must retry the connection three times. If the requests continue to fail, the student will wait for a random period and try again. This simulates a timeout of the segreteria, testing random wait times.

# Development Notes
- **Programming language:** C
- **Platform:** Unix-like
- **Use of sockets**

# Known Bugs
It might occur that, due to the optimization performed by the gcc compiler for MacOs, responses are slower compared to Linux. This behavior is specific to the compiler's optimization for the MacOs operating system.

It's essential to note that this phenomenon is not necessarily an intrinsic error in the code or the program itself but rather a nuance in compiler optimization for different operating systems. The performance difference can be attributed to the specifics of the compiler implementation on MacOs and does not represent a malfunction in the program per se.

# Documentation
A detailed report explaining the implementation aspects is available:

 - [Università_Relazione.pdf](https://github.com/MrPericle/NetworkingProject/files/13928363/Universita_Relazione.pdf)

