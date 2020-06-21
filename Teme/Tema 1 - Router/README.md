Copyright Teodor Matei 323CA - 2020


______            _            
| ___ \          | |           
| |_/ /___  _   _| |_ ___ _ __ 
|    // _ \| | | | __/ _ \ '__|
| |\ \ (_) | |_| | ||  __/ |   
\_| \_\___/ \__,_|\__\___|_|   
                               
                               

*Implementare*
--------------
Implementarea s-a realizat pe un sistem cu Ubuntu 18.04,
gcc 7.5.0. Am folosit headerele standard din Linux pentru
Ethernet, IP si ICMP.

*Parsarea tabelei de rutare*
----------------------------
Este realizata in functia read_table.

*LPM*
-----
Am sortat tabela de rutare la preprocesare: O(nlogn).
Pentru gasirea celei mai bune rute am folosit cautare binara pe biti(O(logn)). In practica, se comporta mai bine decat cea standard (undeva de 4 ori mai rapida).

*Procesul de dirijare*
----------------------
Am respectat pasii din enuntul temei, implementand fix ce am facut in laborator.

*ICMP*
------
- Daca primesc un pachet IP si ICMP, fiind destinat routerului, schimb sursa cu destinatia, setez tipul pachetului in ICMP_ECHOREPLY si trimit pachetul pe interfata.
- Pentru cazurile TTL, respectiv DEST_UNREACH creez un pachet nou in care populez campurile necesare, singura diferenta dintre cele 2 fiind type-ul.
(11 vs 3)

*CHECKSUM*
----------
Am folosit functia care ni s-a oferit in laborator.

*FEEDBACK*
O tema interesanta, greseala mea fiind ca m-am apucat foarte tarziu de tema, am intampinat probleme cand am incercat sa implementez protocolul de ARP(fie nu populam campurile cum trebuie, fie aveam momente cand testele treceau si cand nu), asa ca am renuntat la el.

Una peste alta, KUDOS guys!