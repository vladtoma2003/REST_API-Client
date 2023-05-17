
# Tema 4 Protocoale de comunicatii
In aceasta tema am avut de implementat un client care interactioneaza cu REST API. 

Acest client web este construit in C++. El interactioneaza cu un REST API expus de un server prin protocoliul HTTP. 
Clientul este de tip CLI(Command Line Interface). El primeste niste comenzi de la tastatura si trimite cereri catre server in functie de comanda data.

## Lista de comenzi posibile:
1) register
2) login
3) enter_library
4) get_books
5) get_book
6) add_book
7) delete_book
8) logout
9) exit

## Descrierea implementarii comenzilor
#### 1) REGISTER  

Aceasta comanda creeaza un utilizator nou pe server. Request-ul este de tip *POST REQUEST*.  
Am inceput prin citirea de la tastatura a username-ului si a parolei. Apoi am verificat ca aceste date sa fie valide(sa nu contina spatii).
Am creeat mesajul folosind functia *compute_post_request*, apoi deschid conexiunea cu serverul si salvez file descriptorul socketului 
si trimit mesajul catre server folosind *send_to_server*. Dupa trimiterea mesajului iau raspunsul folosind *recieve_from_server* si interpretez acest mesaj.
Exista 2 posibilitati: Eroare(userul exista deja) sau Succes. Pentru cazul cu eroare se afiseaza un mesaj de eroare iar la succes se afiseaza un mesaj de succes.
La final inchid conexiunea cu serverul. 

#### 2) LOGIN

Aceasta comanda este folosita pentru autentificare pe server. Request-ul este de tip *POST REQUEST*.
Asemanator cu *REGISTER*, se verifica datele de intrare (username-ul si parola), se deschide conexiunea cu serverul, se genereaza mesajul
care urmeaza sa fie trimis catre server si se preia raspunsul de la server. Diferenta este raspunusl. Raspunsul poate fi din nou o eroare,
unde se afiseaza un mesaj de eroare sau succes, unde se afiseaza un mesaj de succes si se salveaza **Cookie**-ul primit. Acest cookie este
al utilizatorului si dovedeste faptul ca userul este conectat. De asemenea cu acest cookie server-ul stie cu cine comunica. La final se 
inchide conexiunea cu server-ul.

#### 3) ENTER_LIBRARY

Aceasta comanda este folosita pentru a accesa toata biblioteca de carti. Request-ul este de tip *GET REQUEST*.
Pentru inceput se verifica daca userul este autentificat. Daca userul nu este autentificat se afiseaza un mesaj de eroare si nu se continua rularea
acestei comenzi. In caz afirmativ, se deschide conexiunea cu serverul si se genereaza mesajul. Acest mesaj va fi generat folosind functia
*compute_get_request*. Apoi se trimite catre server si se preia raspunsul. In acest raspuns se afla un **tokenJWT** care este salvat. Acest
token dovedeste accesul la modificarea si vizionarea cartilor din interiorul bibliotecii. La final se inchide conexiunea cu serverul.

#### 4) GET_BOOKS

Aceasta comanda afiseaza lista tuturor cartilor la care utilizatorul curent are acces. Request-ul este de tip *GET REQUEST*.
Aici se verifica intai daca userul are acces la biblioteca(folosind **token_JWT**) si daca este autentificat. Daca da se deschide conexiunea
cu serverul, se genereaza mesajul si se trimite. Apoi se preia raspunsul si se afiseaza lista de carti(daca nu exista erori). La final se inchide conexiunea cu serverul.

#### 5) GET_BOOK

Aceasta comanda primeste ca parametru in id si intoarce detalii despre cartea cu acel id. Request-ul este de tip *GET REQUEST*.
La inceput am verificat ca id-ul sa fie numar natural (id-ul se trece la finalul rutei de acces). Apoi am verificat daca userul
 este autentificat si are acces. Am deschis conexiunea cu serverul, am generat mesajul si l-am trimis. Apoi am luat raspunsul 
 si am afisat detaliile din **JSON**-ul primit(in raspuns se afla un **JSON** ce continea toate detaliile cartii cerute).
La final am inchis conexiunea cu serverul.

#### 6) ADD_BOOK

Aaceasta comanda adauga o carte pe server. Request-ul este de tip *POST REQUEST*.
Am inceput prin verificarea userului (autentificat si are access). Apoi am facut un **JSON** cu detaliile cartii care urmeaza sa fie adaugata.
Am generat mesajul si am deschis conexiunea cu serverul. Apoi am trimis mesajul, si am luat raspunsul. Am verificat raspunsul si am afisat un mesaj corespunzator.
La final am inchis conexiunea cu serverul.

#### 7) DELETE_BOOk

Aceasta comanda sterge o carte de pe server. Request-ul este de tip *DELETE REQUEST*.
Singura diferenta dintre **get_book** si **delete_book** este tipul de request. De asemenea mesajul este creat folosind
*compute_delete_request*. In rest sunt aceeasi pasi in aceeasi ordine.

#### 8) LOGOUT

Aceasta comanda sterge accesul userului curent si apoi sterge **Cookie**-ul salvat. Request-ul este de tip *GET REQUEST*.
In aceasta comanda se deschide conexiunea, se genereaza mesajul si se trimite. Se ia raspunsul apoi se verifica daca este eroare. Daca nu este
eroare se sterge **tokenJWT** si **Cookie**-ul salvat. La final se inchide conexiunea cu serverul.

#### 9) EXIT
Aceasta comanda este folosita pentru a inchide clientul. In aceasta comanda se da "break;" din while-ul clientului.

## Functii folosite.

Majoritatea functiilor folosite sunt luate din *laboratorul 9*. Functiile folosite in implementarea clientului sunt definite in header-ul
*helpers.h*.  

    Link laborator: https://pcom.pages.upb.ro/labs/lab9/lecture.html

#### Functii create/modificate:
- is_number: Verifica daca stringul dat ca parametru este numar natural.
- compute_post_request: Aceasta functie este luata din laborator si completata. Am urmat pasii comentati in interior pentru a face functia functionala.
- compute_get_request: Functia a fost luata din laborator si completata. Am urmat pasii comentati pentru a face functia functionala.
- compute_delete_request: Aceasta functe este facuta de mine pentru comanda *delete_book*. Functia este foarte asemanatoare cu compute_get_request. Aceasta schimba doar tipul de request din interior.

Fisierele *json_fwd.hpp* si *json.hpp* sunt pentru folosirea **JSON**-ului in C++.

    Sursa fisiere json:  https://github.com/nlohmann/json

Fisierele din directorul Buffer sunt luate din laborator si folosite la o parte din functiile din laborator folosite in tema.
