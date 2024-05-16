#ifndef _REQUESTS_
#define _REQUESTS_

char *creaza_mesag_get(char *host, char *url, char *cookie, char *token, int e_mesaj_tip_get);

char *creaza_mesaj_post(char *host, char *url, char *tip_payload, char *payload, char *cookie, int trebuie_autorizatie);

#endif
