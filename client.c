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

#define server_ip "34.254.242.81"
#define server_port 8080

int main(int argc, char *argv[]) {
    char *input = calloc(100, sizeof(char));
    char *message;
    char *response;
    char *username = calloc(100, sizeof(char));
    char *password = calloc(100, sizeof(char));
    char *username_logged = calloc(100, sizeof(char));
    char *password_logged = calloc(100, sizeof(char));
    char *jwt_token = calloc(1000, sizeof(char));
    char *cookie = calloc(1000, sizeof(char));
    int connected = 0;
    int sockfd;
    sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);

    while (1) {
        // Se introduce comanda dorita de client
        // Mai multe detalii in README
        fgets(input, 100, stdin);
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "exit") == 0) {
            break;
        } else if (strcmp(input, "register") == 0) {
            memset(username, 0, 100);
            memset(password, 0, 100);
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            // Parsez datele introduse de client in format JSON si le trimit la server
            // Se va deschide o noua conexiune de fiecare data cand se trimite o cerere
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string(root_value);

            message = compute_post_request(server_ip, 
                        "/api/v1/tema/auth/register", 
                        "application/json", 
                        &serialized_string, 
                        1, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *token = strtok(response, "\r\n");

            if (strstr(token, "201 Created") != NULL)
                printf("User '%s' with password '%s' has been registered successfully!\n", username, password);
            else {
                printf("Error: User '%s' already exists!\n", username);
            }
        } else if (strcmp(input, "login") == 0) {
            memset(username, 0, 100);
            memset(password, 0, 100);
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string(root_value);

            message = compute_post_request(server_ip,
                        "/api/v1/tema/auth/login", 
                        "application/json", 
                        &serialized_string, 
                        1, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(cookie, "connect.sid") != NULL || strstr(response, "204 No Content") != NULL) {
                printf("Error: User '%s' is already logged in!\n", username_logged);
                continue;
            } else if (strstr(response, "200 OK") != NULL) {
                printf("User '%s' has been logged in successfully!\n", username);
                char *cookie_ptr = strstr(response, "connect.sid");
                if (cookie_ptr != NULL) {
                    char *token = strtok(cookie_ptr, ";");
                    strcpy(cookie, token);
                    printf("Got the cookie!\n");
                }
                // Salvez datele utilizatorului logat
                strcpy(username_logged, username);
                strcpy(password_logged, password);
                connected = 1;
            } else if (strstr(response, "400 Bad Request") != NULL) {
                if (strstr(response, "No account with this username!") != NULL)
                    printf("Error: No account with this username!\n");
                else if (strstr(response, "Credentials") != NULL)
                    printf("Error: Credentials are not good!\n");
            }
        } else if (strcmp(input, "logout") == 0) {
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(server_ip, 
                        "/api/v1/tema/auth/logout", 
                        NULL, &cookie, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "200 OK") != NULL) {
                printf("User '%s' has been logged out successfully!\n", username_logged);
                // Clientul se deconecteaza, deci se sterg datele de logare
                memset(username_logged, 0, 100);
                memset(password_logged, 0, 100);
                memset(jwt_token, 0, 1000);
                memset(cookie, 0, 1000);
                connected = 0;
            } else if (strstr(response, "400 Bad Request") != NULL) {
                printf("Error: You are not logged in!\n");
            }
        } else if (strcmp(input, "enter_library") == 0) {
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(server_ip, 
                        "/api/v1/tema/library/access", 
                        NULL, &cookie, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "200 OK") != NULL) {
                printf("User '%s' has entered the library!\n", username_logged);
                char *json_start = strstr(response, "\r\n\r\n");
                json_start += 4;
                JSON_Value *root_value = json_parse_string(json_start);
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *token = json_object_get_string(root_object, "token");
                strcpy(jwt_token, token);
                printf("Got the JWT Token!\n");
            } else if (strstr(response, "401 Unauthorized") != NULL || connected != 1) {
                printf("Error: You are not logged in!\n");
            }
        } else if (strcmp(input, "get_books") == 0) {
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(server_ip, 
                        "/api/v1/tema/library/books", 
                        NULL, &cookie, 1, jwt_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *json_start = strstr(response, "\r\n\r\n");
            json_start += 4;
            JSON_Value *root_value = json_parse_string(json_start);

            if (strstr(response, "200 OK") != NULL) {
                printf("Books:\n");
                char *serialized_string = NULL;
                serialized_string = json_serialize_to_string_pretty(root_value);
                printf("%s\n", serialized_string);
            } else if (strstr(response, "401 Unauthorized") != NULL || connected != 1) {
                printf("Error: You are not logged in!\n");
            } else if (strstr(response, "403 Forbidden") != NULL) {
                printf("Error: You are not allowed to access this page!\n");
            } else if (strstr(response, "500 Internal Server Error") != NULL) {
                printf("Error: Internal Server Error! Error when decoding tokenn!\n");
            }
        } else if (strcmp(input, "get_book") == 0) {
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            char id[100];
            printf("id=");
            scanf("%s", id);
            char *url = (char *) malloc(1000 *sizeof(char));
            strcpy(url, "/api/v1/tema/library/books/");
            strcat(url, id);

            message = compute_get_request(server_ip, 
                        url, NULL, & cookie, 1, jwt_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "200 OK") != NULL) {
                char *json_start = strstr(response, "\r\n\r\n");
                json_start += 4;
                JSON_Value *root_value = json_parse_string(json_start);
                char *serialized_string = NULL;
                serialized_string = json_serialize_to_string_pretty(root_value);
                printf("%s\n", serialized_string);
            } else if (strstr(response, "400 Bad Request") != NULL) {
                printf("Error: The id is not int! Please try again!\n");
            } else if (strstr(response, "401 Unauthorized") != NULL || connected != 1) {
                printf("Error: You are not logged in!\n");
            } else if (strstr(response, "403 Forbidden") != NULL) {
                printf("Error: You are not allowed to access this page!\n");
            } else if (strstr(response, "404 Not Found") != NULL) {
                printf("Error: Book not found!\n");
            } else if (strstr(response, "500 Internal Server Error") != NULL) {
                printf("Error: Internal Server Error! Error when decoding tokenn!\n");
            }
            free(url);
        } else if (strcmp(input, "add_book") == 0) {
            char title[200], author[200], genre[100], publisher[200], page_count[50];
            printf("title=");
            fgets(title, 200, stdin);
            title[strlen(title) - 1] = '\0';
            printf("author=");
            fgets(author, 200, stdin);
            author[strlen(author) - 1] = '\0';
            printf("genre=");
            fgets(genre, 100, stdin);
            genre[strlen(genre) - 1] = '\0';
            printf("publisher=");
            fgets(publisher, 200, stdin);
            publisher[strlen(publisher) - 1] = '\0';
            printf("page_count=");
            fgets(page_count, 50, stdin);
            page_count[strlen(page_count) - 1] = '\0';

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            if (strlen(title) > 0)
                json_object_set_string(root_object, "title", title);
            if (strlen(author) > 0)
                json_object_set_string(root_object, "author", author);
            if (strlen(genre) > 0)
                json_object_set_string(root_object, "genre", genre);
            if (strlen(publisher) > 0)
                json_object_set_string(root_object, "publisher", publisher);
            if (strlen(page_count) > 0)
                json_object_set_string(root_object, "page_count", page_count);
            serialized_string = json_serialize_to_string(root_value);

            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request(server_ip, 
                        "/api/v1/tema/library/books", 
                        "application/json", 
                        &serialized_string, 
                        1, &cookie, 1, jwt_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "401 Unauthorized") != NULL || connected != 1) {
                printf("Error: You are not logged in!\n");
            } else if (strstr(response, "200 OK") != NULL) {
                printf("Book added successfully!\n");
            } else if (strstr(response, "400 Bad Request") != NULL) {
                char *json_start = strstr(response, "\r\n\r\n");
                json_start += 4;
                JSON_Value *root_value = json_parse_string(json_start);
                JSON_Object *root_object = json_value_get_object(root_value);
                const char *token = json_object_get_string(root_object, "error");
                printf("Error: %s!\n", token);
            } else if (strstr(response, "403 Forbidden") != NULL) {
                printf("Error: You are not allowed to access this page!\n");
            } else if (strstr(response, "500 Internal Server Error") != NULL) {
                printf("Error: Something Bad Happened! Please try again!\n");
            }
        } else if (strcmp(input, "delete_book") == 0) {
            sockfd = open_connection(server_ip, server_port, AF_INET, SOCK_STREAM, 0);
            char id[100];
            printf("id=");
            scanf("%s", id);
            char *url = (char *) malloc(1000 *sizeof(char));
            strcpy(url, "/api/v1/tema/library/books/");
            strcat(url, id);

            message = compute_delete_request(server_ip, 
                        url, NULL, &cookie, 1, jwt_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            if (strstr(response, "200 OK") != NULL) {
                printf("Book deleted successfully!\n");
            } else if (strstr(response, "401 Unauthorized") != NULL || connected != 1) {
                printf("Error: You are not logged in!\n");
            } else if (strstr(response, "403 Forbidden") != NULL) {
                printf("Error: You are not allowed to access this page!\n");
            } else if (strstr(response, "404 Not Found") != NULL) {
                printf("Error: Book not found!\n");
            } else if (strstr(response, "500 Internal Server Error") != NULL) {
                printf("Error: Something Bad Happened! Please try again!\n");
            }
            free(url);
        }
    }
    // Se elibereaza memoria alocata
    free(input);
    free(username);
    free(password);
    free(username_logged);
    free(password_logged);
    free(jwt_token);
    free(cookie);
    free(message);
    free(response);

    close(sockfd);
    return 0;
}