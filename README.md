# IPK-projekt-1

## Informace o projektu a spuštění

Cílem projektu je vytvoření klienta pro vzdálenou komunikaci s kalkulačkou (serverem) pomocí protokolů TCP a UDP.

Po kompilaci příkazem make, se program spouští příkazem:

```ipkcpc -h <host> -p <port> -m <mode>```

Kde host je IPv4 adresa serveru, port je jeho port a mode je zvolený mód pro komunikaci - tcp nebo udp

  

## Obsluha programu

### UDP

Klient čeká na zadání příkladu uživatelem, a to ve formátu:
```(+ 2 3)```
Pro ukončení programu se používá zkratka Ctrl + C, kdy program ukončí spojení a vypne se.
 

### TCP

Nejdříve je potřeba správně navázat komunikaci se serverem pomocí příkazu ```HELLO```.

Poté už lze posílat jednotlivé příklady, zde ve formátu:

```SOLVE (+ 2 (+ 2 3))```

Pro ukončení programu lze použít buď příkaz ```BYE```, nebo opět zkratka Ctrl + C.

  

## Implementace

Program podporuje libovolné pořadí argumentů, avšak musí být dodrženo pořadí přepínač - argument (např. -h 0.0.0.0).

Po zpracování argumentů se zavolá funkce prepare_connect, která zjistí požadovanou IP adresu z DNS serveru a následně vytvoří socket pro daný protokol.

Při úspěšném vytvoření socketu se volá funkce do_udp nebo do_tcp podle zadaného módu.

  

### do_ucp

Funkce zajišťující spojení pomocí UDP protokolu. Převezme zadanou adresu a načte vstup uživatele, poté složí zprávu podle "Request Message Format" serveru. To zajišťuje tato část kódu:

```

fgets(input, BUFSIZE, stdin);

char send[257] = {0, (char)strlen(input)};

memcpy(send + 2, input, 255);

```

Tedy na prvních 8 bitech se nachází 0 pro požadavek, na dalších 8 bitech se nachází délka odesílané zprávy přetypovaná na char a ve zbytku pole jsou data zadaná uživatelem. To znamená, že načtená zpráva by neměla přesáhnout délku 255.

Data se odešlou na server a proběhne příjem a následná kontrola dat ze serveru pro vypsání výsledku či chyby.

Odesílání dat je zacykleno, tudíž lze poslat příklad kolikrát je libo. Při ukončení pomocí Ctrl + C se provede uzavření socketu a ukončení programu.

  

### do_tcp

Funkce funguje na podobném principu jako UDP, avšak je upravená na TCP protokol. Na začátku proběhne spojení se serverem, dále ve smyčce probíhá odesílání zpráv na server, jejich příjem a vypsání. Pokud server zaregistruje neočekávaný či špatně zadaný vstup, posílá zprávu BYE, na kterou program reaguje ukončením spojení se serverem, uzavřením socketu a vlastním ukončením. Na ukončení Ctrl + C program reaguje odesláním BYE serveru a přijetím BYE ze serveru. Poté opět ukončením spojení, uzavřením socketu a ukončením.

  

### Komentář k některým částem implementace

Makefile pouze kompiluje zdrojový kód, nebo po sobě uklízí.

Neimplementoval jsem timeout pro odesílání a přijímaní dat serveru, nechávám dobu čekání na uživateli - zadání nespecifikuje.

  

## Testování

Program jsem testoval nejdříve lokálně běžícím referenčním serverem na Ubuntu a následně i přímo na referenčním OS nix.

### UDP příklad testu
 Ukončení pomocí CTRL + C
```
(+ 2 3) 	//vstup
OK:5		//vystup
kk
ERR:Could not parse the message
(+ 2 (+ 2 3))
OK:7
^C
```  

### TCP příklad testu
 Ukončení pomocí CTRL + C
```
HELLO
HELLO
SOLVE (+ 2 (+ 2 (/ 86 105)))
RESULT 506/105
^CBYE
BYE
``` 
Ukončení pomocí zprávy BYE
```
HELLO
HELLO
SOLVE (+ 2 (+ 2 (* 69 (- 420 88))))
RESULT 22912
BYE 
BYE
```  


## Zdroje

Při implementaci jsou využil následující zdroje:

https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp/client.c

https://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event