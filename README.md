# Tema 4 - Client Web. Comunicaţie cu REST API.

Teodorescu Ioan, 323CB

Tema consta in implementarea a unui client Web, care sa comunica cu un REST API. Acesta are ca 
principiu modelul clasic “client-server”, in care clientul trimite un **request**, iar serverul un 
**response**. Insa, de aceasta data ne aflam in contextul web-ului modern, care foloseste protocolul 
HTTP.

- Pentru rezolvarea temei, m-am folosit de scheletul Laboratorului 9
- Pentru parsarea JSON, am folosit `parson` [https://github.com/kgabis/parson], care prezinta functii
de serializare, de formatare, de creere si de extragere a datelor din continuturile de tip JSON, care
sunt usor de folosit! Pentru folosirea lor (a functiilor), am folosit exemplele puse pe pagina lor de
 github.

Pentru inceput, voi explica functiile de computare a request-urilor de tip GET, POST, DELETE din 
`requests.c`

`compute_get_request` = GET este folosit pentru a extrage date dintr-un anumit server. 

**Request Line** = Incep prin a scrie numele metodei (GET), dorind sa gasim resurse. Apoi, adaug 
URL-ul de unde doresc a gasii acele resurse (alaturi de parametrii de interogare `[query_params]` daca 
este necesar). La final adaug protocolul care va fi folosit pentru request (HTTP 1.1).


**Request Header** = Se adauga metadatele care vor fi trimise in request pentru a aduce informatii 
despre acesta. Vom adauga urmatoarele:

1. Host-ul = domeniul server-ului
2. Cookies (optional) = sunt folosite pentru managementul sesiunii dintre server si client. 
3. Token-ul JWT (optional) = e folosit pentru transmiterea informatiilor intre client si server, fara
 ca un potential atacator sa modifice datele.

Se mai adauga o linie finala.

`compute_post_request` - POST trimite date catre server. 

**Request Line** = Asemanator ca GET

**Request Header =** Este adaugat Host-ul, Token-ul JWT (optional), tipul continului dat de resurse 
(in acest caz,`"application/json"`), Dimensiunea resurselor, Cookies (optional) si resursele in sine, 
in format JSON. 

`compute_delete_request` - DELETE sterge resursele specificate de client din server.

- Implementarea este la fel ca functiile de mai sus, doar ca adaugam doar:
    - Numele metodei - DELETE
    - URL, query_parameters (optional), Numele protocolului
    - Host-ul
    - Token-ul JWT (optional)
    - Cookies (optional)

### Client.c

Se aloca variabile pentru: nume, prenume (cele cu logged sunt folosite pentru utilizatorul care s-a
 logat la server), token jwt si cookie. Connected este folosit pentru confirmarea faptului ca avem un 
 utilizator conectat.

Intr-un loop, program asteapta sa scriem comenzi, iar pentru fiecare din aceasta se procedeaza astfel:

1. `Register` - efectueaza inregistrarea
    - Se ofera un prompt pentru username si password
    - Datele sunt serializate in format JSON
    - Se trimit catre server (POST request) si se primeste un raspuns
    - Dupa raspuns, lucrurile se desfasoara astfel
    1. `201 Created` = s-a creat utilizatorul
    
    In orice alt caz, inseamna ca utilizatorul cu acel username exista deja
    
2. `Login` - efectueaza autentificarea
    - Se ofera prompt pentru username si password
    - Datele sunt serializate in format JSON
    - Se trimit catre server (POST request) si se primeste un raspuns
    - Dupa raspuns, lucrurile se desfasoara astfel
    1. `200 OK` = Utilizatorul s-a autentificat cu succes. Se extrage cookie-ul primit ca raspuns si 
    se va memora, alaturi de username-ul si password-ul clientului logat. Connected se va face 1, 
    intrucat avem o conexiune intre client si server.
    2. `400 Bad Request` - Sunt 2 cazuri; Unul in care nu avem un utilizator cu acel username si cel 
    in care nu am introdus datele (credentials) corect. Pentru fiecare mesaj se printeaza la output un 
    mesaj de eroare.
3. `Logout` - efectueaza logout
    - Se trimite un GET request catre server, la ruta `/api/v1/tema/auth/logout`
    - Dupa raspuns, lucrurile se desfasoara astfel
    1. `200 OK` = Utilizatorul s-a deconectat de la server cu succes. Se vor reseta variabilele care
     memorau informatii despre acesta, inclusiv connected va deveni 0, intrucat nu mai avem un client
      activ. 
    2. `400 Bad Request` = Nu exista un utilizator logat; Se va afisa un mesaj de eroare privind acest 
    lucru
4. `Enter_library` - cere acces in biblioteca
    - Se trimite un GET request catre server, alaturi de cookie-ul primit la autentificare.
    - Dupa raspuns, lucrurile se desfasoara astfel
    - `200 OK` = Accesul in biblioteca s-a efectuat cu succes. Se elimina tot header-ul, pentru a lua 
    token-ul jwt, folosind functiile de parsare json. Se va memora si acest token.
    - `401 Unauthorized` sau `connected != 1` = Focus-ul principal se baseaza pe faptul ca noi nu avem 
    un user autentificat ori pentru ca nu avem cookie ori pentru ca nu am setat variabila connected. Se va afisa un mesaj de eroare
5. `Get_books` - cere toate cartile de pe server
    - Vom primi cartile create de contul conectat la server
    - Se face un GET request
    - Dupa raspuns, lucrurile se desfasoara astfel
    1. `200 OK` - Totul a functionat ok. Se vor lua datele primite, se formateaza si se vor afisa cartile.
    2. `401 Unauthorized` - nu avem un utilizator autentificat; nu este permis!
    3. `403 Forbidden` - nu avem acces la pagina; header-ul de Authorization nu a fost pus in request
    4. `500 Internal Server Error` - eroare de la server; Token-ul JWT nu a fost trimis cum trebuie
6. `Get_book` - cere o carte dupa id
    - Se ofera un prompt pentru id
    - Se trimite requestul la ruta `/api/v1/tema/library/books/:bookId` si se primeste raspunsul
    - Dupa raspuns, lucrurile se desfasoara astfel
    1. `200 OK` - Totul a functionat OK! Se extrag datele, se formateaza si se vor afisa
    2. `400 Bad Request` - Id-ul nu este un numar
    3. `404 Not Found` - cartea respectiva nu exista in server.
    4. `403 Forbidden` - nu avem acces la pagina; header-ul de Authorization nu a fost pus in request
    5. `500 Internal Server Error` - eroare de la server; Token-ul JWT nu a fost trimis cum trebuie
7. `Add_book` - adauga o carte in server.
    - Se ofera un prompt pentru titlu, autor, gen, editor, numar de pagini. Se verifica daca in 
    variabile sunt efectiv date
    - Datele sunt serializate in format JSON
    - Se trimit catre server (POST request) si se primeste un raspuns
    1. `200 OK` - Totul a functionat OK! Cartea a fost adaugata cu succes!
    2. `400 Bad Request` - Unul dintre variabile nu a fost introdus in carte. Trebuie sa readaugam 
    cartea cu TOATE variabilele.
    3. `403 Forbidden` - nu avem acces la pagina; header-ul de Authorization nu a fost pus in request
    4. `500 Internal Server Error` - eroare de la server; Token-ul JWT nu a fost trimis cum trebuie. 
    Un alt caz pentru care ar da aceasta eroare este atunci cand numarul de pagini nu este un int.

    
8. `Delete_book` - sterge o carte din server.
    
    - Se ofera un prompt pentru id
    - Se trimite requestul cu metoda DELETE la ruta `/api/v1/tema/library/books/:bookId` si se primeste raspunsul
    - Dupa raspuns, lucrurile se desfasoara astfel
    1. `200 OK` - Totul a functionat OK! Cartea a fost stearsa cu succes.
    2. `401 Unauthorized` - nu avem un utilizator autentificat; nu este permis!
    3. `403 Forbidden` - nu avem acces la pagina; header-ul de Authorization nu a fost pus in request
    4. `404 Not Found` - cartea respectiva nu exista in server.
    5. `500 Internal Server Error` - eroare de la server; Token-ul JWT nu a fost trimis cum trebuie. Un alt caz pentru care ar da aceasta eroare este atunci cand id-ul nu este un int.
    

In asta consta implementearea temei. Punctajul de pe checker este acesta: 

```bash
python3 checker.py --user 'testoriginal:123' ./client

register: 
    User 'testoriginal' with password '123' has been registered successfully!
login: 
    User 'testoriginal' has been logged in successfully!
    Got the cookie!
enter_library: 
    User 'testoriginal' has entered the library!
    Got the JWT Token!
get_books: 
    Retrieved book IDs + titles: 
    []
    []
add_book: 
    Book added successfully!
add_book: 
    Book added successfully!
get_books: 
    Retrieved book IDs + titles: 
    ['10648', '10649']
    ['Computer Networks', 'Viata Lui Nutu Camataru: Dresor de Lei si de Fraieri']
    OKAY: count=2
get_book_id: 
    OKAY: fields match!
delete_book: 
    Book deleted successfully!
logout: 
    User 'testoriginal' has been logged out successfully!
exit:
```