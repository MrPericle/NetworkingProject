# Networking Project
The project aims to develop a parallel client/server application to manage university exams. The outline involves the creation of three groups of students, each responsible for a specific part of the application.

# Manuale Utente per Compilazione ed Esecuzione

## Clone della Repository

Per ottenere il codice sorgente del progetto, è necessario clonare la repository da GitHub. Utilizzare il seguente comando da terminale:

```
 git clone https://github.com/MrPericle/NetworkingProject.git
```
Successivamente, spostarsi nella directory appena clonata tramite il comando:
```
 cd NetworkingProject
```

## Compilazione
Il progetto è composto da tre file eseguibili distinti: **serverUniversitario**, **segreteria**, e **client**. Ogni file deve essere compilato separatamente, come indicato di seguito:

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

## Esecuzione
Dopo la compilazione, eseguire gli eseguibili nell'ordine seguente:

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

## Funzionalità
Una volta avviati i componenti, il server universitario rimarrà in ascolto. Le funzionalità principali della segreteria e dello studente sono le seguenti:

### Segreteria:
1. Inserisce gli esami sul server dell'università.
2. Inoltra le richieste di prenotazione degli studenti al server universitario.
3. Fornisce allo studente le date degli esami disponibili per il corso selezionato.
   
### Studente:
1. Chiede alla segreteria se ci sono esami disponibili per un determinato corso.
2. Invia una richiesta di prenotazione di un esame alla segreteria.

## Funzionalità aggiuntive
- Il server universitario, ad ogni richiesta di prenotazione, invia alla segreteria il numero di prenotazione progressivo assegnato allo studente, e la segreteria lo inoltra allo studente.

- Se la segreteria non risponde alla richiesta dello studente, questo dovrà ritentare la connessione per 3 volte. Se le richieste continuano a fallire, allora aspetterà un periodo di tempo casuale e riproverà. Questo simula un timeout della segreteria, testando l'attesa casuale.


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
