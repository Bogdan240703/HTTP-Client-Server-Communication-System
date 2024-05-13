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
    sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
    char comanda[15];
    char nume_user[15];
    char parola[15];
    char **cookies = NULL;
    while (1)
    {
        scanf("%s", comanda);
        if (strcmp(comanda, "register") == 0)
        {
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
            printf("Payload JSON: %s\n", json_string);
            // message=compute_post_request("34.246.184.49","/api/v1/tema/auth/register","application/json",,2,NULL,0);
            // send_to_server(sockfd,message);
            printf("200 - OK");
        }
        if (strcmp(comanda, "login") == 0)
        {
            printf("user=");
            scanf("%s", nume_user);
            printf("password=");
            scanf("%s", parola);
        }
    }
    close_connection(sockfd);
    return 0;
}
