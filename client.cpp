#include <stdio.h>
#include <iostream>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "Utils/json.hpp"
#include "Utils/json_fwd.hpp"
#include "Utils/helpers.h"

#define IP (char*)"34.254.242.81"
#define PORT 8080
#define IP_PORT (char*)"34.254.242.81:8080"

int main() {

    char* message;
    char* response;
    int sockfd;

    int logged_in = 0; // ce se intampla daca sunt logat si dau register?
    std::string userCookie;

    int access = 0;
    std::string tokenJWT;

    while(1) {
        std::string command;
        std::getline(std::cin, command);
        
        if(command == "register") { // merge cu == pt ca e string
            // POST REEQUEST
            std::string username;
            std::cout << "username:";
            std::getline(std::cin,  username);

            std::string password;
            std::cout << "password:";
            std::getline(std::cin,  password);

            nlohmann::json user = {
                {"username", username},
                {"password", password}
            };

            // verific daca username ul sau parola contin spatiu
            if(std::string(username).find(' ') != std::string::npos || std::string(password).find(' ') != std::string::npos) {
                std::cout << "Error: Bad username or password input!" << std::endl;
                continue;
            }

            // generez mesajul de register
            message = compute_post_request(
                IP_PORT,
                (char *)"/api/v1/tema/auth/register",
                (char *)"application/json",
                user.dump(),
                1,
                NULL,
                0
            );

            // deschid conexiunea cu serverul
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            // verific daca am primit eroare
            if(strstr(response, "error")) {
                std::cout << "Error: The username " << username << " is taken!\n";
            } else {
                std::cout << "User added succesfully!" << std::endl;
            }

            // inchid conexiunea cu serverul
            close_connection(sockfd);


        }else if (command == "login") {
            // POST REQUEST
            std::string username;
            std::cout << "username:";
            std::getline(std::cin,  username);

            std::string password;
            std::cout << "password:";
            std::getline(std::cin,  password);

            nlohmann::json user = {
                {"username", username},
                {"password", password}
            };

            // verific daca username ul sau parola contin spatiu
            if(std::string(username).find(' ') != std::string::npos || std::string(password).find(' ') != std::string::npos) {
                std::cout << "Error: Bad username or password input!" << std::endl;
                continue;
            }

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            // generez mesajul de login
            message = compute_post_request(
                IP_PORT,
                (char *)"/api/v1/tema/auth/login",
                (char *)"application/json",
                user.dump(),
                1,
                NULL,
                0
            );

            // trimit cerere login
            send_to_server(sockfd, message);

            // primesc raspuns login
            response = receive_from_server(sockfd);

            // verific daca am primit eroare
            if(strstr(response, "error")) {
                std::cout << "Error: Credentials are not good!" << std::endl;
            } else {
                std::cout << "Login succesful!" << std::endl;
                logged_in = 1;

                unsigned int start = std::string(response).find("Cookie: ");
                unsigned int end = std::string(response).find(";", start);

                userCookie = std::string(response).substr(start, end - start);
            }

            // inchid conexiunea cu serverul
            close_connection(sockfd);

        } else if(command == "enter_library") {
            // GET REQUEST
            if(logged_in == 0) {
                std::cout << "Error: You are not logged in!" << std::endl;
                continue;
            }

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            // fac cookies
            std::string cookies[10];
            cookies[0] = userCookie;

            // generez mesajul de get_access
            message = compute_get_request(
                IP_PORT,
                (char *)"/api/v1/tema/library/access",
                NULL,
                cookies,
                1
            );

            // trimit cererea
            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);
            if(strstr(response, "error")) { // nu ar trebui sa intre pe cazul asta
                std::cout << "Error: You are not logged in!" << std::endl;
            } else {
                std::cout << "Access granted!" << std::endl;
                access = 1;

                unsigned int start = std::string(response).find("token"); // {"token":"..."}
                unsigned int end = std::string(response).find("}", start);

                tokenJWT = "Authorization: Bearer " + 
                                    std::string(response).substr(start + strlen("token") + 3, // 3 pt ":"
                                                         end - start - strlen("token") - 4); // 4 pt ":"}
            }

            // inchid conexiunea cu sv
            close_connection(sockfd);

        } else if(command == "get_books") {
            // GET REQUEST

            if(!access) { // verific daca am access la carti
                std::cout << "Error: Authorization header is missing!" << std::endl;
                continue;
            }

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            std::string tokens[10];
            tokens[0] = tokenJWT;

            // generez mesajul de get_books
            message = compute_get_request(
                IP_PORT,
                (char *)"/api/v1/tema/library/books",
                NULL,
                tokens,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: Authorization header is missing!" << std::endl;
            } else {
                unsigned int start = std::string(response).find("[");
                unsigned int end = std::string(response).find("]", start);

                std::string books = std::string(response).substr(start, end - start + 1);
                nlohmann::json carti = nlohmann::json::parse(books);
                
                for(nlohmann::json carte : carti) {
                    std::cout << "id: " << carte["id"] << ", titlu: " << carte["title"] << std::endl;
                }

            }

        } else if(command == "get_book") {
            // GET REQUEST
            if(!access) {
                std::cout << "Error: Authorization header is missing!" << std::endl;
                continue;
            }

            std::string id;
            std::cout << "id:";
            std::getline(std::cin,  id);

            if(!is_number(id)) {
                std::cout << "Error: Bad id input!" << std::endl;
                continue;
            }

            // verific daca id ul este numar (functia returneaza 0 si daca acesta contine spatiu)
            if(!is_number(id)) {
                std::cout << "Error: Bad id input!" << std::endl;
                continue;
            }

            std::string api = "/api/v1/tema/library/books/" + id;

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            std::string tokens[10];
            tokens[0] = tokenJWT;

            // generez mesajul de get_book
            message = compute_get_request(
                IP_PORT,
                (char *)api.c_str(),
                NULL,
                tokens,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: ID doesn't exist!" << std::endl;
            } else {
                unsigned int start = std::string(response).find("{"); // retunreaza o lista de un json
                unsigned int end = std::string(response).find("}", start);

                std::string book = std::string(response).substr(start, end - start + 1); // un string json
                nlohmann::json carte = nlohmann::json::parse(book); // transform stringul in json

                std::cout << "titlu: " << carte["title"] << std::endl
                 << "autor: " << carte["author"] << std::endl
                 << "genre: " << carte["genre"] << std::endl
                 << "page count: " << carte["page_count"] << std::endl
                 << "publisher: " << carte["publisher"] << std::endl;
            }

        } else if(command == "add_book") {
            // POST REQUEST
            if(!access) {
                std::cout << "Error: Authorization header is missing!" << std::endl;
                continue;
            }

            std::string title;
            std::cout << "title:";
            // std::cin.ignore();
            std::getline(std::cin, title);

            std::string author;
            std::cout << "author:";
            // std::cin.ignore();
            std::getline(std::cin, author);

            std::string genre;
            std::cout << "genre:";
            // std::cin.ignore();
            std::getline(std::cin, genre);

            std::string page_count;
            std::cout << "page count:";
            std::getline(std::cin, page_count);

            if(!is_number(page_count)) {
                std::cout << "Error: Bad page count input!" << std::endl;
                continue;
            }

            std::string publisher;
            std::cout << "publisher:";
            // std::cin.ignore();   
            std::getline(std::cin, publisher);

            nlohmann::json book = {
                {"title", title},
                {"author", author},
                {"genre", genre},
                {"page_count", page_count},
                {"publisher", publisher}
            };

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            std::string tokens[10];
            tokens[0] = tokenJWT;

            message = compute_post_request(
                IP_PORT,
                (char *)"/api/v1/tema/library/books",
                (char *)"application/json",
                book.dump(),
                1,
                tokens,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: Wrong Format" << std::endl;
            } else {
                std::cout << "Book added!" << std::endl;

            }

            // inchid conexiunea cu sv
            close_connection(sockfd);

        } else if (command == "delete_book") {
            // DELETE REQUEST
            if(!access) {
                std::cout << "Error: Authorization header is missing!" << std::endl;
                continue;
            }

            std::string id;
            std::cout << "id:";
            std::getline(std::cin,  id);

            // verific daca id ul este numar (functia returneaza 0 si daca acesta contine spatiu)
            if(!is_number(id)) {
                std::cout << "Error: Bad id input!" << std::endl;
                continue;
            }

            std::string api = "/api/v1/tema/library/books/" + id;

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            std::string tokens[10];
            tokens[0] = tokenJWT;

            message = compute_delete_request(
                IP_PORT,
                (char *)api.c_str(),
                NULL,
                tokens,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: ID doesn't exist!" << std::endl;
            } else {
                std::cout << "Book deleted!" << std::endl;
            }

        } else if(command == "logout") {
            if(!logged_in) {
                std::cout << "Error: You are not logged in!" << std::endl;
                continue;
            }

            std::string cookies[10];
            cookies[0] = userCookie;

            // deschid conexiunea cu sv
            sockfd = open_connection(IP, PORT, PF_INET, SOCK_STREAM, 0);

            message = compute_get_request(
                IP_PORT,
                (char *)"/api/v1/tema/auth/logout",
                NULL,
                cookies,
                1
            );

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd);

            if(strstr(response, "error")) {
                std::cout << "Error: You are not logged in!" << std::endl;
            } else {
                std::cout << "Logout successful!" << std::endl;
                logged_in = 0;
                access = 0;
                tokenJWT = "";
                userCookie = "";
            }

        } else if (command == "exit") {
            break; // vreau sa ies
        }
    }

    return 0;
}
