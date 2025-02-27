// SAVU BOGDAN - 322 CB

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

// Variabile globale de care am nevoie in mai multe functii
char *mesaj_de_trimis;
char *raspuns_server;
int sockfd;
char *comanda;
char *nume_user;
char *parola;
char *cookie;
char *token;
char *id;
char *titlu, autor, gen, publicatie, nr_pagini;

char *creaza_string_json(char **campuri, char **val_campuri, int nr_campuri)
{
    // Aici initializez obiectul json asa cum apare in githubul ce era postat
    // in enuntul temei
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    // Aici adaug campurile primite
    for (int i = 0; i < nr_campuri; i++)
    {
        json_object_set_string(root_object, campuri[i], val_campuri[i]);
    }
    // Fac serializarea pentru a il obtine sub forma de string
    char *json_string = json_serialize_to_string(root_value);
    return json_string;
}

int verifica_raspuns_OK(char *raspuns_server)
{
    // daca codul intors in raspuns incepe cu 2 inseamna ca e ok, altfel nu
    if (raspuns_server[9] == '2')
        return 1;
    else
        return 0;
}

int executa_comanda_exit()
{
    // pregatesc programul pt inchidere
    printf("SUCCES - Inchidere program!\n");
    return 0;
}

void rezolva_comanda_resgister()
{
    // intai citesc rescul de informatii
    printf("username=");
    scanf("%s", nume_user);
    printf("password=");
    scanf("%s", parola);
    // aici imi formez datele pentru a le putea pune in json
    char **campuri = malloc(2 * sizeof(char *));
    campuri[0] = "username";
    campuri[1] = "password";
    char **val_campuri = malloc(2 * sizeof(char *));
    val_campuri[0] = nume_user;
    val_campuri[1] = parola;
    // creez jsonul
    char *json_string = creaza_string_json(campuri, val_campuri, 2);
    // eliberez campurile de care nu mai am nevoie, intrucat am info-ul stocat in json
    free(campuri);
    free(val_campuri);
    // creez mesajul ce trebuie trimis catre server
    mesaj_de_trimis = creaza_mesaj_post("34.246.184.49", "/api/v1/tema/auth/register", "application/json", json_string, NULL, 0);
    // il trimit la server
    send_to_server(sockfd, mesaj_de_trimis);
    // astept sa primesc raspuns
    raspuns_server = receive_from_server(sockfd);
    // in functie de raspunsul primit afisez succes sau eroare
    if (verifica_raspuns_OK(raspuns_server) == 1)
        printf("SUCCES: User-ul a fost inregistrat cu succes!\n");
    else
    {
        printf("ERROR User-ul nu a fost inregistrat!:\n%s\n", raspuns_server);
    }
}

void rezolva_comanda_enter_library()
{
    // aici incerc direct sa formatez mesajul ce trebuie trimis la server, punand si cookie de aceasta data
    // intrucat trebuie sa deonstram ca suntem logati
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", "/api/v1/tema/library/access", cookie, NULL, 1);
    // il trimit la server
    send_to_server(sockfd, mesaj_de_trimis);
    // astept sa primesc raspunsul
    raspuns_server = receive_from_server(sockfd);

    if (verifica_raspuns_OK(raspuns_server) == 1)
    {
        // aici raspusnul e ok, asa ca incep sa extrag tokenul primit
        printf("SUCCES ! Utilizatorul are acces la biblioteca!\n");
        int j = 0;
        // incerc sa mi dau seama unde incepe tokenul
        while (raspuns_server[j] != '{')
            j++;
        j += 10;
        int i = 0;
        // aici stiu ca sunt la inceutul lui, si merg pana la sfarsit copiand element cu element
        // tokenul din raspuns in variabila token, in care il voi salva
        while (raspuns_server[j] != '"')
        {
            token[i++] = raspuns_server[j];
            j++;
        }
        token[i] = '\0';
    }
    else
    {
        // aici inseamna ca ceva nu a mers bine
        printf("ERROR ! Accesara bibliotecii nu s-a putut realiza:\n%s\n", raspuns_server);
    }
}

void rezolva_comanda_get_books()
{
    // creez mesajul ce trebuie trimis
    // aici pun token-ul pentru a demonstra ca am acces la biblioteca
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", "/api/v1/tema/library/books", NULL, token, 1);
    // trimit la server
    send_to_server(sockfd, mesaj_de_trimis);
    // astept raspuns
    raspuns_server = receive_from_server(sockfd);
    if (verifica_raspuns_OK(raspuns_server) == 0)
    {
        // daca nu e ok afisez eroare
        printf("ERROR ! Nu aveti acces la biblioteca!\n%s\n", raspuns_server);
    }
    else
    {
        // daca e ok parcurg mesajul pana unde incepe payload-ul
        int i = 0;
        while (raspuns_server[i] != '[')
        {
            i++;
        }
        // retin indicele unde incepe, si afisez de acolo pana la sfarsitul mesajului primit
        // (acesta fiind chiar payload-ul cu cartile care ne intereseaza)
        printf("%s\n", raspuns_server + i);
    }
}

void rezolva_comanda_login()
{
    // aici citesc si restul de informatii de care am nevoie
    printf("username=");
    scanf("%s", nume_user);
    printf("password=");
    scanf("%s", parola);
    // la fel ca la register formez json-ul cu aceste informatii
    char **campuri = malloc(2 * sizeof(char *));
    campuri[0] = "username";
    campuri[1] = "password";
    char **val_campuri = malloc(2 * sizeof(char *));
    val_campuri[0] = nume_user;
    val_campuri[1] = parola;
    char *json_string = creaza_string_json(campuri, val_campuri, 2);
    // eliberez campurile de care nu mai am nevoie
    free(campuri);
    free(val_campuri);
    // creez mesajul
    mesaj_de_trimis = creaza_mesaj_post("34.246.184.49", "/api/v1/tema/auth/login", "application/json", json_string, NULL, 0);
    // il trimit
    send_to_server(sockfd, mesaj_de_trimis);
    // astept raspuns de la server
    raspuns_server = receive_from_server(sockfd);
    if (verifica_raspuns_OK(raspuns_server) == 1)
    {
        // daca e ok rapsunsul incerc sa extrag cookieul
        printf("SUCCES - login-ul a fost realizat!\n");
        int j = 0;
        // aici vad de unce incepe campul cu cookieul
        while (raspuns_server[j] != '\0')
        {
            if (raspuns_server[j] == 's')
            {
                if (raspuns_server[j + 1] == 'i')
                    if (raspuns_server[j + 2] == 'd')
                    {
                        break;
                    }
            }
            j++;
        }
        // aici stiu unde incepe, si copiez element cu element in variabila cookie, pentrua  il retine
        char *aux = raspuns_server + j;
        aux += 4;
        int i = 0;
        while (aux[i] != ';')
        {
            cookie[i] = aux[i];
            i++;
        }
        cookie[i] = '\0';
    }
    else
    {
        // afisez mesaj de eroare daca nu a fost ok
        printf("ERROR Login-ul a esuat:\n%s\n", raspuns_server);
    }
}

void rezolva_comanda_get_book()
{
    // citesc si id-ul cartii
    printf("id=");
    scanf("%s", id);
    // imi construiesc url-ul catre carte asa cum este mentionat in enunt
    char *line = calloc(LINELEN, sizeof(char));
    sprintf(line, "/api/v1/tema/library/books/%s", id);
    // construiesc cererea
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", line, NULL, token, 1);
    free(line);
    // o trimit la server
    send_to_server(sockfd, mesaj_de_trimis);
    // astept rapsunsul serverului
    raspuns_server = receive_from_server(sockfd);
    if (verifica_raspuns_OK(raspuns_server) == 1)
    {
        // daca  e ok atunci parcurg mesajul primit paa la inceperea payload-ului care ma
        // intereseaza, acesta fiind in format json va incepe la '{'
        int i = 0;
        while (raspuns_server[i] != '{')
        {
            i++;
        }
        // afisez informatiile despre carte primite de la server
        printf("%s\n", raspuns_server + i);
    }
    else
    {
        printf("ERROR :Cartea cu id=%s nu exista sau nu aveti accesa la biblioteca:\n%s!\n", id, raspuns_server);
    }
}

void rezolva_comanda_delete_book()
{
    // citesc id-ul cartii ce trebuie stearsa
    printf("id=");
    scanf("%s", id);
    char *line = calloc(LINELEN, sizeof(char));
    sprintf(line, "/api/v1/tema/library/books/%s", id);
    // pun 0 tipul mesajului pt ca trebuie sa fie comanda de delete
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", line, NULL, token, 0);
    free(line);
    // trimit mesajul la server
    send_to_server(sockfd, mesaj_de_trimis);
    // astept raspuns
    raspuns_server = receive_from_server(sockfd);
    // verific daca comanda a reusit sau nu
    if (verifica_raspuns_OK(raspuns_server) == 1)
    {
        printf("SUCCES - Cartea cu id=%s a fost stearsa cu succes!\n", id);
    }
    else
    {
        printf("ERROR - Cartea cu id=%s nu exista sau nu aveti acces la biblioteca!\n%s\n", id, raspuns_server);
    }
}

void rezolva_comanda_logout()
{
    // creez mesajul de logout pe care sa il trimit la server
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", "/api/v1/tema/auth/logout", cookie, NULL, 1);
    send_to_server(sockfd, mesaj_de_trimis);
    // astept raspunsul
    raspuns_server = receive_from_server(sockfd);
    if (verifica_raspuns_OK(raspuns_server) == 1)
    {
        // daca e ok printes mesaj de succes
        printf("SUCCES - Utilizatorul s-a delogat cu succes!\n");
    }
    else
    {
        // altfel printez eroare
        printf("ERROR - A aparut o eroare! Nu sunteti autentificat\n");
    }
}

int rezolva_comanda_add_book()
{
    // aici citesc restul de informatii despre cartea pe care trebuie sa o adaug
    char titlu[101], autor[55], gen[55], publicatie[55], nr_pagini[15];
    printf("title=");
    // consum newlineul
    getchar();
    // citesc linia
    fgets(titlu, sizeof(titlu), stdin);
    int len = strlen(titlu);
    /*
     * apoi elimin newline-ul de la final daca acesta exista
     * pentrua nu aparea in numele cartii
     * apoi repet acest proces pentru toate celelalte
     * informatii pe care le citesc
     */
    if (len > 0 && titlu[len - 1] == '\n')
    {
        titlu[len - 1] = '\0';
    }
    printf("author=");
    fgets(autor, sizeof(autor), stdin);
    len = strlen(autor);
    if (len > 0 && autor[len - 1] == '\n')
    {
        autor[len - 1] = '\0';
    }
    printf("genre=");
    fgets(gen, sizeof(gen), stdin);
    len = strlen(gen);
    if (len > 0 && gen[len - 1] == '\n')
    {
        gen[len - 1] = '\0';
    }
    printf("publisher=");
    fgets(publicatie, sizeof(publicatie), stdin);
    len = strlen(publicatie);
    if (len > 0 && publicatie[len - 1] == '\n')
    {
        publicatie[len - 1] = '\0';
    }
    printf("page_count=");
    scanf("%s", nr_pagini);
    int j = 0, ok = 1;
    // aici verific si daca numarul de pagini este chiar numar, si nu un string
    while (nr_pagini[j])
    {
        if (nr_pagini[j] >= '0' && nr_pagini[j] <= '9')
        {
            j++;
        }
        else
        {
            ok = 0;
            break;
        }
    }
    if (ok == 0)
    {
        // daca e strign afisez mesaj de eroare si opresc executia comenzii
        printf("ERROR - Valoarea introdusa pentru nr_pagini nu reprezinta un numar !!\n");
        return 0;
    }
    // aici construiesc jsonul
    char **campuri = malloc(5 * sizeof(char *));
    campuri[0] = "title";
    campuri[1] = "author";
    campuri[2] = "genre";
    campuri[3] = "page_count";
    campuri[4] = "publisher";
    char **val_campuri = malloc(5 * sizeof(char *));
    val_campuri[0] = titlu;
    val_campuri[1] = autor;
    val_campuri[2] = gen;
    val_campuri[3] = nr_pagini;
    val_campuri[4] = publicatie;
    char *json_string = creaza_string_json(campuri, val_campuri, 5);
    free(campuri);
    free(val_campuri);
    // creez mesajul de post
    mesaj_de_trimis = creaza_mesaj_post("34.246.184.49", "/api/v1/tema/library/books", "application/json", json_string, token, 1);
    send_to_server(sockfd, mesaj_de_trimis);
    // astept raspuns de la server
    raspuns_server = receive_from_server(sockfd);
    if (verifica_raspuns_OK(raspuns_server) == 1)
    {
        printf("SUCCES - Cartea a fost adaugata cu succes!\n");
    }
    else
    {
        printf("ERROR - Informatiile despre carte nu respecta formatarea sau nu aveti acces la biblioteca:\n%s\n", raspuns_server);
    }
    return 1;
}

int verifica_tip_comanda(char *comanda)
{
    // aici inseamna ca am primit o comanda, asa ca deschid conexiunea cu serverul
    sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
    // verific ce fel de comanda e si apelez mai departe functia corespunzatoare
    if (strcmp(comanda, "register") == 0)
    {
        rezolva_comanda_resgister();
    }
    if (strcmp(comanda, "login") == 0)
    {
        rezolva_comanda_login();
    }
    if (strcmp(comanda, "enter_library") == 0)
    {
        rezolva_comanda_enter_library();
    }
    if (strcmp(comanda, "get_books") == 0)
    {
        rezolva_comanda_get_books();
    }
    if (strcmp(comanda, "get_book") == 0)
    {
        rezolva_comanda_get_book();
    }
    if (strcmp(comanda, "add_book") == 0)
    {
        rezolva_comanda_add_book();
    }
    if (strcmp(comanda, "delete_book") == 0)
    {
        rezolva_comanda_delete_book();
    }
    if (strcmp(comanda, "logout") == 0)
    {
        rezolva_comanda_logout();
    }
    if (strcmp(comanda, "exit") == 0)
    {
        // daca e exit inchid conexiunea inainte sa dau return
        // pentru a nu ramane dechisa
        close_connection(sockfd);
        return executa_comanda_exit();
    }
    // aici inchid conexiunea dupa executia uneie dintre celelalte comenzi in afara de exit
    close_connection(sockfd);
    return 1;
}

int main(int argc, char *argv[])
{
    comanda = malloc(15 * sizeof(char));
    nume_user = malloc(30 * sizeof(char));
    parola = malloc(30 * sizeof(char));
    cookie = malloc(256 * sizeof(char));
    token = malloc(300 * sizeof(char));
    id = malloc(15 * sizeof(char));
    while (1)
    {
        scanf("%s", comanda);
        /*
         * Apelez direct functia care verifica de fel de comanda e
         * aceasta ar trebui sa intoarca 0 al comanda de exit, caz in care dau
         * break pentru a putea sa opresc in mod normal rularea programului,
         * eliberand si memoria
         */
        if (verifica_tip_comanda(comanda) == 0)
            break;
    }
    free(comanda);
    free(nume_user);
    free(parola);
    free(cookie);
    free(token);
    free(id);
    return 0;
}
