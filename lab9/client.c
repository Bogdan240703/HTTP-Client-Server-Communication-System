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
int executa_comanda_exit()
{
    printf("SUCCES - Inchidere program!\n");
    return 0;
}
char *creaza_string_json(char **campuri, char **val_campuri, int nr_campuri)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    // Adaugă câmpuri la obiectul JSON
    for (int i = 0; i < nr_campuri; i++)
    {
        json_object_set_string(root_object, campuri[i], val_campuri[i]);
    }
    // Serializare obiectului JSON într-un șir de caractere JSON
    char *json_string = json_serialize_to_string(root_value);
    return json_string;
}
void rezolva_comanda_resgister()
{
    char **campuri = malloc(2 * sizeof(char *));
    campuri[0] = "username";
    campuri[1] = "password";
    char **val_campuri = malloc(2 * sizeof(char *));
    val_campuri[0] = nume_user;
    val_campuri[1] = parola;
    char *json_string = creaza_string_json(campuri, val_campuri, 2);
    free(campuri);
    free(val_campuri);
    mesaj_de_trimis = creaza_mesaj_post("34.246.184.49", "/api/v1/tema/auth/register", "application/json", json_string, NULL, 0);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    if (raspuns_server[9] == '2')
        printf("SUCCES: User-ul a fost inregistrat cu succes!\n");
    else
    {
        printf("ERROR User-ul nu a fost inregistrat!:\n%s\n", raspuns_server);
    }
}
void rezolva_comanda_enter_library()
{
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", "/api/v1/tema/library/access", cookie, NULL, 1);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    if (raspuns_server[9] == '2')
    {
        printf("SUCCES ! Utilizatorul are acces la biblioteca!\n");
        int j = 0;
        while (raspuns_server[j] != '{')
            j++;
        j += 10;
        int i = 0;
        while (raspuns_server[j] != '"')
        {
            token[i++] = raspuns_server[j];
            j++;
        }
        token[i] = '\0';
    }
    else
    {
        printf("ERROR ! Accesara bibliotecii nu s-a putut realiza:\n%s\n", raspuns_server);
    }
}
void rezolva_comanda_get_books()
{
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", "/api/v1/tema/library/books", NULL, token, 1);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    if (raspuns_server[9] != '2')
    {
        printf("ERROR ! Nu aveti acces la biblioteca!\n%s\n", raspuns_server);
    }
    else
    {
        int i = 0;
        while (raspuns_server[i] != '[')
        {
            i++;
        }
        printf("%s\n", raspuns_server + i);
    }
}
void rezolva_comanda_login()
{
    char **campuri = malloc(2 * sizeof(char *));
    campuri[0] = "username";
    campuri[1] = "password";
    char **val_campuri = malloc(2 * sizeof(char *));
    val_campuri[0] = nume_user;
    val_campuri[1] = parola;
    char *json_string = creaza_string_json(campuri, val_campuri, 2);
    free(campuri);
    free(val_campuri);
    mesaj_de_trimis = creaza_mesaj_post("34.246.184.49", "/api/v1/tema/auth/login", "application/json", json_string, NULL, 0);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    int j = 0;
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

    if (raspuns_server[9] == '2')
    {
        printf("SUCCES - login-ul a fost realizat!\n");
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
        printf("ERROR Login-ul a esuat:\n%s\n", raspuns_server);
    }
}
void rezolva_comanda_get_book()
{
    char *line = calloc(LINELEN, sizeof(char));
    sprintf(line, "/api/v1/tema/library/books/%s", id);
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", line, NULL, token, 1);
    free(line);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    if (raspuns_server[9] == '2')
    {
        int i = 0;
        while (raspuns_server[i] != '{')
        {
            i++;
        }
        printf("%s\n", raspuns_server + i);
    }
    else
    {
        printf("ERROR :Cartea cu id=%s nu exista sau nu aveti accesa la biblioteca:\n%s!\n", id, raspuns_server);
    }
}
void rezolva_comanda_delete_book()
{
    char *line = calloc(LINELEN, sizeof(char));
    sprintf(line, "/api/v1/tema/library/books/%s", id);
    // pun cookie count 0 pt ca trb sa fie comanda de delete
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", line, NULL, token, 0);
    free(line);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    // printf("raspuns: %s\n", raspuns_server);
    if (raspuns_server[9] == '2')
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
    mesaj_de_trimis = creaza_mesag_get("34.246.184.49", "/api/v1/tema/auth/logout", cookie, NULL, 1);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    if (raspuns_server[9] == '2')
    {
        printf("SUCCES - Utilizatorul s-a delogat cu succes!\n");
    }
    else
    {
        printf("ERROR - A aparut o eroare! Nu sunteti autentificat\n");
    }
}
int rezolva_comanda_add_book()
{
    char titlu[101], autor[55], gen[55], publicatie[55], nr_pagini[15];
    printf("title=");
    getchar();
    fgets(titlu, sizeof(titlu), stdin);
    int len = strlen(titlu);
    if (len > 0 && titlu[len - 1] == '\n')
    {
        titlu[len - 1] = '\0';
    }
    printf("author=");
    fgets(autor, sizeof(autor), stdin);
    printf("genre=");
    fgets(gen, sizeof(gen), stdin);
    printf("publisher=");
    fgets(publicatie, sizeof(publicatie), stdin);
    printf("page_count=");
    scanf("%s", nr_pagini);
    int j = 0, ok = 1;
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
        printf("ERROR - Valoarea introdusa pentru nr_pagini nu reprezinta un numar !!\n");
        return 0;
    }
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
    mesaj_de_trimis = creaza_mesaj_post("34.246.184.49", "/api/v1/tema/library/books", "application/json", json_string, token, 1);
    send_to_server(sockfd, mesaj_de_trimis);
    raspuns_server = receive_from_server(sockfd);
    if (raspuns_server[9] == '2')
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
    sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
    if (strcmp(comanda, "register") == 0)
    {
        printf("username=");
        scanf("%s", nume_user);
        printf("password=");
        scanf("%s", parola);
        rezolva_comanda_resgister();
    }
    if (strcmp(comanda, "login") == 0)
    {
        printf("username=");
        scanf("%s", nume_user);
        printf("password=");
        scanf("%s", parola);
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
        printf("id=");
        scanf("%s", id);
        rezolva_comanda_get_book();
    }
    if (strcmp(comanda, "add_book") == 0)
    {
        rezolva_comanda_add_book();
    }
    if (strcmp(comanda, "delete_book") == 0)
    {
        printf("id=");
        scanf("%s", id);
        rezolva_comanda_delete_book();
    }
    if (strcmp(comanda, "logout") == 0)
    {
        rezolva_comanda_logout();
    }
    if (strcmp(comanda, "exit") == 0)
    {
        close_connection(sockfd);
        return executa_comanda_exit();
    }
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
