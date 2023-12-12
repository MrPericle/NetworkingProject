# Networking Project
The project aims to develop a parallel client/server application to manage university exams. The outline involves the creation of three groups of students, each responsible for a specific part of the application.

## Secretary
**Tasks:**
1. Enters exams on the university server.
2. Forwards students' reservation requests to the university server.
3. Provides students with available exam dates for their chosen course.

## University Server
**Tasks:**
1. Receives the addition of new exams.
2. Receives the reservation of an exam.

## Timeout Management
**Tasks:**
1. If the secretary does not respond to the student's request, the student must retry the connection three times.
2. If requests continue to fail, the student waits for a random time and retries.
3. Simulate a timeout from the secretary to test random waiting.

# Implementation
Below, the implementation of various points is detailed.

## Secretary
...

## University Server
...

## Timeout Management
...

# Development Notes
- **Programming language:** C
- **Platform:** Unix-like
- **Use of sockets**

# Documentation
A detailed report explaining the implementation aspects is available:

 - ...

# Contributions
- **Camilla De Martino:** [Insert contributions]
- **Lorenzo Pergmao:** [Insert contributions]
- **Carmine Coppola:** [Insert contributions]
