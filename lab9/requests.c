#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
void seteaza_tip_cerere_corespunzator(int tip_mesaj, char *line, char *url, char *mesaj)
{
    // aici pun fie metoda DELETE fie GET
    if (tip_mesaj == 0)
    {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    else if (tip_mesaj == 1)
    {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    else
    {
        sprintf(line, "POST %s HTTP/1.1", url);
    }
    compute_message(mesaj, line);
}
// Am facut functii pentru campurile care erau emreu setate atat in post cat si in get/delete
void adauga_camp_autorizare(char *line, char *token, char *mesaj)
{
    sprintf(line, "Authorization: Bearer %s", token);
    compute_message(mesaj, line);
}
void adauga_camp_host(char *line, char *host, char *mesaj)
{
    sprintf(line, "Host: %s", host);
    compute_message(mesaj, line);
}
char *creaza_mesag_get(char *host, char *url, char *cookie, char *token, int e_mesaj_tip_get)
{
    char *mesaj = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    // adaug numele metodei, impreuna cu url-ul in header
    seteaza_tip_cerere_corespunzator(e_mesaj_tip_get, line, url, mesaj);

    // apoi adaug host-ul, adica adresa ip
    adauga_camp_host(line, host, mesaj);
    // apoi in functie daca am primit sau nu cookie sau token le pun in header sau nu
    if (cookie != NULL)
    {
        // formatez inapoi cookie-ul asa cum a venit
        sprintf(line, "Cookie: connect.sid=%s", cookie);
        compute_message(mesaj, line);
    }
    if (token != NULL)
    {
        // aici pun token-ul de autorizatie asa cum se mentioneaza in enuntul temei
        adauga_camp_autorizare(line, token, mesaj);
    }
    // adaug linia goala de dupa header
    compute_message(mesaj, "");
    free(line);
    return mesaj;
}

char *creaza_mesaj_post(char *host, char *url, char *tip_payload, char *payload, char *cookie, int trebuie_autorizatie)
{
    char *mesaj = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    // adaug numele metodei, impreuna cu url-ul in header
    seteaza_tip_cerere_corespunzator(2, line, url, mesaj);

    // apoi linia cu hostul, adica adresa ip
    adauga_camp_host(line, host, mesaj);
    // verific daca e nevoie de token in mesaj
    if (trebuie_autorizatie == 1)
    {
        // de fapt e token aici
        adauga_camp_autorizare(line, cookie, mesaj);
    }
    // adaug tipul continutului
    sprintf(line, "Content-Type: %s", tip_payload);
    compute_message(mesaj, line);
    // adauga si lungimea continutului
    sprintf(line, "Content-Length: %d", (int)strlen(payload));
    compute_message(mesaj, line);
    // adaug linia de separare intre header si payload
    compute_message(mesaj, "");
    // si aici adaug si payload-ul
    compute_message(mesaj, payload);

    free(line);
    return mesaj;
}
