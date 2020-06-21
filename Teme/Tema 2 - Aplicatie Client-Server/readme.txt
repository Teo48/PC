Copyright Teodor Matei 323CA - 2020


 _____ _ _            _     _____                          
/  __ \ (_)          | |   /  ___|                         
| /  \/ |_  ___ _ __ | |_  \ `--.  ___ _ ____   _____ _ __ 
| |   | | |/ _ \ '_ \| __|  `--. \/ _ \ '__\ \ / / _ \ '__|
| \__/\ | |  __/ | | | |_  /\__/ /  __/ |   \ V /  __/ |   
 \____/_|_|\___|_| |_|\__| \____/ \___|_|    \_/ \___|_|   
                                                           
                                                           

*Subscriber*
-----------
Fisierele subscriber.cpp, subscriber_utils.cpp contin implementarea clientului de TCP.

Creez un socket de comunicare cu serverul, dezactivand algoritmul lui Neagle si
multiplexez cu STDIN pentru face cereri catre server(comenzi de tip subscribe,
unsubscribe si exit).

In momentul in care un client primeste un mesaj de la server, acesta receptioneaza
prima data dimensiunea mesajului, urmata de continutul acestuia. Am asociat primii
2 bytes pentru lungime, urmatorul byte pentru tipul mesajului(daca este INT, FLOAT,
SHORT_REAL sau STRING), iar apoi continutul efectiv.

Subscriber-ul se inchide in momentul in care nu mai primeste octeti de la server
sau este introdusa comanda exit.

Pentru tratarea erorilor:
	- Cele critice sunt tratate cu ajutorul macrourilor MEM_ALOC si ASSERT ce afiseaza
	la stderr un mesaj de eroare, elibereaza memoria si inchid executia subscriberului.

	- Erorile generate de catre utilizator, de pilda introducerea gresita a comenzilor
	se trateaza prin afisarea unui mesaj de eroare la stderr, ignorarea comenzii si
	asteptarea introducerii unei alte comenzi.

*Server*
-------
Fiserele server.cpp, server_utils.cpp contin implementarea serverului.
Acesta gestioneaza informatii despre ce mesaje trebuiesc transmise, despre
clientii online, cui trebuiesc transmise, respectiv salvarea mesajelor pentru
clientii ce au SF setat pe 1.

Mesajele pentru clientii ce au SF setat sunt stocate in fisere binare(avand
urmatorul format: ID_CLIENT_NUME_TOPIC) in
momentul in care acestia se deconecteaza, la reconectare fiindu-le transmise
toate aceste mesaje dupa care continutul este sters pentru a eficientiza
procesul (Am considerat ca este mai eficient asa din vreme ce cantitatea de
stocare este de regula mult mai mare decat memoria RAM disponibila),
iar pastrarea datelor ce vizeaza clientii se realizeaza cu ajutorul map-ului
din STL, deoarece acesta permite accesare in timp constant O(1).

Pentru a gestiona toate inputurile(de la clientii UDP si TCP) se foloseste
multiplexarea I/O. Creez socketi UDP si TCP, cel TCP fiind pasiv, pe care
se asculta cereri de la noii subscriberi.

Cand serverul primeste un mesaj de la un client UDP, acesta va atasa
mesajului adresa ip, respectiv portul clientului UDP, urmand ca apoi
acesta sa fie parsat pentru a stii exact cati octeti trebuiesc trimisi.

In cazul unei cereri de la un client TCP avem urmatoarele posibilitati:
	- S-a conectat un client nou (Intrarea din map-ul ce tine evidenta
	clientilor este macarcata ca fiind Unassigend)
	- A primit o cerere de tip subscribe / unsubscribe ce trebuie procesata.

Cand se primeste o cerere de tip subscribe, ea se executa indiferent daca
clientul este abonat sau nu, doarece map pastreaza ultima intrare, fapt
pentru care daca clientul avea SF setat pe 1 si s-a deconecat, iar le 
reconectare il seteaza pe 0 el nu va mai fi notificat la o deconectare
si o reconectare ulterioara.

Serverul primeste o singura comanda, si anume exit, in cazul in care
sunt introduse orice alte comenzi, utilizatorul este atentionat.

Executia serverului incheie in momentul introducerii comenzii exit.

Modalitati de rulare:
./server <PORT>
./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>

Comenzi acceptate de clienti:
- subscribe topic [SF] - SF 1 pentru a retine mesajele, 0 in caz contrar
- unsubcribe topic
- exit


Pentru tratarea erorilor:
	- Cele critice sunt tratate cu ajutorul macrourilor MEM_ALOC si ASSERT ce afiseaza
	la stderr un mesaj de eroare, elibereaza memoria si inchid executia subscriberului.

	- Erorile generate de catre clientii UDP, se trateaza prin afisarea unui mesaj de
	eroare la stderr si ignorarea acestora, programul continuandu-si executia.