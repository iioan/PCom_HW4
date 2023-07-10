#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
    char **cookies, int cookies_count, char *authorization_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }
        compute_message(message, line);
    }

    // Step 4 (optional): add authorization header if provided
    if (authorization_token != NULL && strcmp(authorization_token, "") != 0) {
        sprintf(line, "Authorization: Bearer %s", authorization_token);
        compute_message(message, line);
    }

    // Step 5: add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
    int body_data_fields_count, char **cookies, int cookies_count, char *authorization_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2 (optional): add authorization header if provided
    if (authorization_token != NULL && strcmp(authorization_token, "") != 0) {
        sprintf(line, "Authorization: Bearer %s", authorization_token);
        compute_message(message, line);
    }

    // Step 3: add the content type
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // Step 4: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 5: add necessary headers - Content-Length 
    // compute content length first
    int body_data_len = 0;
    for (int i = 0; i < body_data_fields_count; i++) {
        body_data_len += strlen(body_data[i]);
    }

    sprintf(line, "Content-Length: %d", body_data_len);
    compute_message(message, line);

    // Step 6 (optional): add cookies
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }
        compute_message(message, line);
    }

    // Step 7: add a new line at the end of the header
    compute_message(message, "");

    // Step 8: add the actual payload data
    for (int i = 0; i < body_data_fields_count; i++) {
        strcat(body_data_buffer, body_data[i]);
    }

    compute_message(message, body_data_buffer);

    // Clean up and return the message
    free(line);
    free(body_data_buffer);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
    char **cookies, int cookies_count, char *authorization_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3: add authorization header if provided
    if (authorization_token != NULL && strcmp(authorization_token, "") != 0) {
        sprintf(line, "Authorization: Bearer %s", authorization_token);
        compute_message(message, line);
    }

    // Step 4 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }
        compute_message(message, line);
    }

    // Step 5: add final new line
    compute_message(message, "");

    free(line);
    return message;
}