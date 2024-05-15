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
int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char comanda[15];
    char nume_user[15];
    char parola[15];
    char cookie[256];
    char token[300];
    char **cookies = NULL;
    while (1)
    {

        // setvbuf(stdout, NULL, _IONBF, BUFSIZ);
        scanf("%s", comanda);
        if (strcmp(comanda, "register") == 0)
        {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            printf("username=");
            scanf("%s", nume_user);
            printf("password=");
            scanf("%s", parola);
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            // Adaugă câmpuri la obiectul JSON
            json_object_set_string(root_object, "username", nume_user);
            json_object_set_string(root_object, "password", parola);

            // Serializare obiectului JSON într-un șir de caractere JSON
            char *json_string = json_serialize_to_string(root_value);

            // Afisează șirul JSON
            // printf("Payload JSON: %s", json_string);
            message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/register", "application/json", json_string, 2, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (response[9] == '2')
                printf("200 - OK\n");
            else
            {
                printf("ERROR: %s\n", response);
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "login") == 0)
        {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            printf("username=");
            scanf("%s", nume_user);
            printf("password=");
            scanf("%s", parola);
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            // Adaugă câmpuri la obiectul JSON
            json_object_set_string(root_object, "username", nume_user);
            json_object_set_string(root_object, "password", parola);

            // Serializare obiectului JSON într-un șir de caractere JSON
            char *json_string = json_serialize_to_string(root_value);

            // Afisează șirul JSON
            message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/login", "application/json", json_string, 2, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printf("aici: %s", response);
            int j = 0;
            while (response[j] != '\0')
            {
                if (response[j] == 's')
                {
                    if (response[j + 1] == 'i')
                        if (response[j + 2] == 'd')
                        {
                            break;
                        }
                }
                j++;
            }
            char *aux = response + j;
            aux += 4;
            int i = 0;
            while (aux[i] != ';')
            {
                cookie[i] = aux[i];
                i++;
            }
            cookie[i] = '\0';
            if (response[9] == '2')
                printf("200 - OK\n");
            else
            {
                printf("ERROR: %s\n", response);
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "enter_library") == 0)
        {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.246.184.49", "/api/v1/tema/library/access", NULL, cookie, NULL, 1);
            // printf("%s", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (response[9] == '2')
            {
                printf("Utilizatorul are acces la biblioteca\n");
                int j = 0;
                while (response[j] != '{')
                    j++;
                j += 10;
                int i = 0;
                while (response[j] != '"')
                {
                    token[i++] = response[j];
                    j++;
                }
                token[i] = '\0';
            }
            else
            {
                printf("ERROR: %s\n", response);
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "get_books") == 0)
        {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", NULL, NULL, token, 1);
            // printf("%s", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int i = 0;
            while (response[i] != '[')
            {
                i++;
            }
            printf("%s\n", response + i);
            close_connection(sockfd);
        }
        if (strcmp(comanda, "get_book") == 0)
        {
            char id[15];
            printf("id=");
            scanf("%s", id);
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *line = calloc(LINELEN, sizeof(char));
            sprintf(line, "/api/v1/tema/library/books/%s", id);
            message = compute_get_request("34.246.184.49", line, NULL, NULL, token, 1);
            free(line);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (response[9] == '2')
            {
                int i = 0;
                while (response[i] != '{')
                {
                    i++;
                }
                printf("%s\n", response + i);
            }
            else
            {
                printf("Cartea cu id=%s nu exista!\n", id);
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "add_book") == 0)
        {
            char titlu[85], autor[55], gen[55], publicatie[55], nr_pagini[15];
            printf("title=");
            getchar();
            fgets(titlu, sizeof(titlu), stdin);

            // Remove trailing newline character, if present
            size_t len = strlen(titlu);
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
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            // Adaugă câmpuri la obiectul JSON
            json_object_set_string(root_object, "title", titlu);
            json_object_set_string(root_object, "author", autor);
            json_object_set_string(root_object, "genre", gen);
            json_object_set_string(root_object, "page_count", nr_pagini);
            json_object_set_string(root_object, "publisher", publicatie);

            // Serializare obiectului JSON într-un șir de caractere JSON
            char *json_string = json_serialize_to_string(root_value);
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            // message = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", NULL, NULL, token, 1);
            message = compute_post_request("34.246.184.49", "/api/v1/tema/library/books", "application/json", json_string, 1, token, 1);
            // printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printf("%s\n", response);
            if (response[9] == '2')
            {
                printf("OK - Cartea a fost adaugata cu succes!\n");
            }
            else
            {
                printf("Eroare - Cartea nu a fost adaugata\n");
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "delete_book") == 0)
        {
            char id[15];
            printf("id=");
            scanf("%s", id);
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *line = calloc(LINELEN, sizeof(char));
            sprintf(line, "/api/v1/tema/library/books/%s", id);
            // pun cookie count 0 pt ca trb sa fie comanda de delete
            message = compute_get_request("34.246.184.49", line, NULL, NULL, token, 0);
            free(line);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printf("raspuns: %s\n", response);
            if (response[9] == '2')
            {
                printf("Cartea cu id=%s a fost stearsa cu succes!\n", id);
            }
            else
            {
                printf("Cartea cu id=%s nu exista!\n", id);
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "logout") == 0)
        {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("34.246.184.49", "/api/v1/tema/auth/logout", NULL, cookie, NULL, 1);
            // printf("%s", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if (response[9] == '2')
            {
                printf("Utilizatorul s-a delogat cu succes!\n");
            }
            else
            {
                printf("A aparut o eroare! Nu sunteti autentificat\n");
            }
            close_connection(sockfd);
        }
        if (strcmp(comanda, "exit") == 0)
        {
            printf("Inchidere program!\n");
            break;
        }
    }
    return 0;
}
