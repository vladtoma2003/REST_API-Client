#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

/*
 * @param msg - the message to be shown
 * Used for the functions below.
 */
void error(const char *msg);

/*
 * @param message - the message to be sent
 * @param line - the line to be added
 * Adds a line to the string message.
 */
void compute_message(char *message, const char *line);

/*
 * @param host_ip - the ip of the host
 * @param portno - the port number
 * @param ip_type - the type of the ip
 * @param socket_type - the type of the socket
 * @param flag - the flag
 * Opens a connection with server host_ip on port portno, returns a socket.
 */
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

/*
 * @param sockfd - the socket to be closed
 * Closes a server connection on socket sockfd.
 */
void close_connection(int sockfd);

/*
 * @param sockfd - the socket to be used
 * @param message - the message to be sent
 * Send a message to a server.
 */
void send_to_server(int sockfd, char *message);

/*
 * @param sockfd - the socket to be used
 * Receives and returns the message from a server.
 */
char *receive_from_server(int sockfd);

/*
 * @param str - the string to be processed
 * Extracts and returns a JSON from a server response.
 */
char *basic_extract_json_response(char *str);

/*
 * @param str - the string to be processed
 * Verifies if the given string is a number.
 */
int is_number(std::string str);

// Request functions:

/*
 * @param host - the host to be used
 * @param url - the url to be used
 * @param content_type - the type of the content
 * @param body_data - the body data
 * @param body_data_fields_count - the number of fields
 * @param cookies - the cookies to be used
 * @param cookies_count - the number of cookies
 * Computes and returns a POST request string (cookies can be NULL if not needed)
 */
char *compute_post_request(char *host, char *url, char *content_type, std::string body_data,
                           int body_data_fields_count, std::string *cookies, int cookies_count);

/*
 * @param host - the host to be used
 * @param url - the url to be used
 * @param query_params - the query parameters
 * @param cookies - the cookies to be used
 * @param cookies_count - the number of cookies
 * Computes and returns a GET request string (query_params and cookies can be set to NULL if not needed).
 */
char *compute_get_request(char *host, char *url, char *query_params, std::string *cookies,
                          int cookies_count);

/*
 * @param host - the host to be used
 * @param url - the url to be used
 * @param query_params - the query parameters
 * @param cookies - the cookies to be used
 * @param cookies_count - the number of cookies
 * Computes and returns a DELETE request string (query_params and cookies can be NULL if not needed).
 */
char *compute_delete_request(char *host, char *url, char *query_params, std::string *cookies,
                             int cookies_count);


#endif
