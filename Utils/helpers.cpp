#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <iostream>
#include "helpers.h"
#include "Buffer/buffer.h"


#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}


char *receive_from_server(int sockfd) {
  char response[BUFLEN];
  buffer buffer = buffer_init();
  int header_end = 0;
  int content_length = 0;

  do {
    int bytes = read(sockfd, response, BUFLEN);

    if (bytes < 0) {
      error("ERROR reading response from socket");
    }

    if (bytes == 0) {
      break;
    }

    buffer_add(&buffer, response, (size_t)bytes);

    header_end =
        buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

    if (header_end >= 0) {
      header_end += HEADER_TERMINATOR_SIZE;

      int content_length_start =
          buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);

      if (content_length_start < 0) {
        continue;
      }

      content_length_start += CONTENT_LENGTH_SIZE;
      content_length = strtol(buffer.data + content_length_start, NULL, 10);
      break;
    }
  } while (1);
  size_t total = content_length + (size_t)header_end;

  while (buffer.size < total) {
    int bytes = read(sockfd, response, BUFLEN);

    if (bytes < 0) {
      error("ERROR reading response from socket");
    }

    if (bytes == 0) {
      break;
    }

    buffer_add(&buffer, response, (size_t)bytes);
  }
  buffer_add(&buffer, "", 1);
  return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

int is_number(std::string s) {
    for (int i = 0; i < s.length(); i++) {
        if (isdigit(s[i]) == false) {
            return 0;
        }
    }
    return 1;
}

// Request functions:
char *compute_post_request(char *host, char *url, char* content_type, std::string body_data,
                            int body_data_fields_count, std::string *cookies, int cookies_count)
{
    char *message = (char*)calloc(BUFLEN, sizeof(char));
    char *line = (char*)calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", body_data.length());
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
      // compute_message(message, "Cookie: ");
      for(int i = 0; i < cookies_count; i++) {
          compute_message(message, cookies[i].c_str());
      }
    }
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);
    compute_message(message, body_data.c_str());

    free(line);
    return message;
}

char *compute_get_request(char *host, char *url, char *query_params,
                            std::string *cookies, int cookies_count)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

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
        for(int i = 0; i < cookies_count; i++) {
            compute_message(message, cookies[i].c_str());
        }
    }
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params, 
                                  std::string *cookies, int cookies_count)
{
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  // Numele metodei, URL, parametrii de request (daca exista) si protocolul
  if(query_params) {
    sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
  } else {
    sprintf(line, "DELETE %s HTTP/1.1", url);
  }  

  compute_message(message, line);

  // adaug hostul
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  // adaug cookies (optional) si/sau alte headere specifice
  if (cookies != NULL) {
    for(int i = 0; i < cookies_count; i++) {
        compute_message(message, cookies[i].c_str());
    }
  }

  // Add final new line
  compute_message(message, "");
  free(line);
  return message;
}
