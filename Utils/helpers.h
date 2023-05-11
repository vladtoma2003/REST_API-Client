#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

//shows the current error
void error(const char *msg);

// adds a line to the string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// verifies if the given string is a number
int is_number(std::string str);

// Request functions:
// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, std::string body_data,
							int body_data_fields_count, std::string *cookies, int cookies_count);

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params, std::string *cookies, int cookies_count);

// computes and returns a DELETE request string (query_params and cookies can be NULL if not needed)
char *compute_delete_request(char *host, char *url, char *query_params, std::string *cookies, int cookies_count);


#endif
