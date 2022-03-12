#include <iostream>
#include <cstring>
#include <zmq.hpp>
#include <string>
#include <thread>
#include <unistd.h>

void send_message(const std::string &message_string, zmq::socket_t &socket) {
    zmq::message_t message_back(message_string.size());
    memcpy(message_back.data(), message_string.c_str(), message_string.size()); // message_t.data() - извлекает указатель на msg, из src в dest
    if (!socket.send(message_back)) {
        std::cout << "Error" << std::endl;
    }
}

std::string receive_message(zmq::socket_t &socket) {
    zmq::message_t message_main;
    socket.recv(&message_main);
    std::string answer(static_cast<char *>(message_main.data()), message_main.size());
    return answer;
}

void process_terminal(zmq::socket_t &pusher, std::string login) {
    std::string command = "";
    std::cout << "Enter command" << std::endl;
    while (std::cin >> command) {
        if (command == "send") {
            std::cout << "Enter nickname of recipient" << std::endl;
            std::string recipient = "";
            std::cin >> recipient;
            std::cout << "Enter your message" << std::endl;
            std::string client_message = "";
            char a;
            std::cin >> a;
            std::getline(std::cin, client_message);
            std::string message_string = "send " + recipient + " " + login + " " + a + client_message;
            send_message(message_string, pusher);
        } else if (command == "gs") {
            std::cout << "Enter group of recipient" << std::endl;
            std::string group = "";
            std::cin >> group;
            std::cout << "Enter your message" << std::endl;
            std::string client_message = "";
            char a;
            std::cin >> a;
            std::getline(std::cin, client_message);
            std::string message_string = "gs " + group + " " + login + " " + a + client_message;
            send_message(message_string, pusher);
        } else if (command == "addgroup") {
            std::cout << "Enter group" << std::endl;
            std::string group = "";
            std::cin >> group;
            std::string message_string = "addgroup " + group + " " + login;
            send_message(message_string, pusher);
        } else if (command == "leavegroup") {
            std::cout << "Enter group" << std::endl;
            std::string group = "";
            std::cin >> group;
            std::string message_string = "leavegroup " + group + " " + login;
        } else if (command == "grouplist") {
            std::string message_string = "grouplist " + login;
            send_message(message_string, pusher);
        } else if (command == "exit") {
            send_message("exit " + login, pusher);
            break;
        }
        std::cout << "Enter command" << std::endl;
    }
}

void process_server(zmq::socket_t &puller, std::string login) {
    while (true) {
        std::string command = "";
        std::string recieved_message = receive_message(puller);
        for (char i: recieved_message) {
            if (i != ' ') {
                command += i;
            } else {
                break;
            }
        }
        if (command == "send") {
            int i;
            std::string recipient = "", sender = "", mes_to_me = "";
            for (i = 5; i < recieved_message.size(); ++i) {
                if (recieved_message[i] != ' ') {
                    recipient += recieved_message[i];
                } else {
                    break;
                }
            }
            ++i;
            for (i; i < recieved_message.size(); ++i) {
                if (recieved_message[i] != ' ') {
                    sender += recieved_message[i];
                } else {
                    break;
                }
            }
            ++i;
            for (i; i < recieved_message.size(); ++i) {
                mes_to_me += recieved_message[i];
            }
            std::cout << "Message from " << sender << ":" << std::endl << mes_to_me << std::endl;
        } else if (command == "gs") {
            int i;
            std::string group = "", sender = "", mes_to_me = "";
            for (i = 3; i < recieved_message.size(); ++i) {
                if (recieved_message[i] != ' ') {
                    group += recieved_message[i];
                } else {
                    break;
                }
            }
            ++i;
            for (i; i < recieved_message.size(); ++i) {
                if (recieved_message[i] != ' ') {
                    sender += recieved_message[i];
                } else {
                    break;
                }
            }
            ++i;
            for (i; i < recieved_message.size(); ++i) {
                mes_to_me += recieved_message[i];
            }
            std::cout << "Message from " << sender << " to " + group + " group:" << std::endl << mes_to_me << std::endl;
        } else if (command == "group") {
            std::cout << recieved_message << std::endl;
        } else if (command == "leaved") {
            std::cout << recieved_message << std::endl;
        } else if (command == "your") {
            std::cout << recieved_message << std::endl;
        } else if (command == "you") {
            std::cout << recieved_message << std::endl;
        } else if (command == "no") {
            std::cout << "We didn`t find this user/group" << std::endl;
        } else if (command == "exit") {
            puller.disconnect("tcp://localhost:3" + std::to_string(getpid()));
            break;
        }
    }
}

//PULL (client) - PUSH (server)
int main() {
    zmq::context_t context(1);
    zmq::socket_t socket_for_login(context, ZMQ_REQ); // для отправки и получения сообщений

    socket_for_login.connect("tcp://localhost:4042");
    std::cout << "Enter login: " << std::endl;
    std::string login = "";
    std::cin >> login;
    std::cout << "Enter group: " << std::endl;
    std::string group = "";
    std::cin >> group;
    send_message(std::to_string(getpid()) + " " + login + " " + group, socket_for_login);
    std::string recieved_message = receive_message(socket_for_login);
    if (recieved_message == "0") {
        std::cout << "login is already used" << std::endl;
        _exit(0);
    } else if (recieved_message == "1") {
        zmq::context_t context1(1);
        zmq::socket_t puller(context1,
                             ZMQ_PULL); // Сокет типа ZMQ_PULL используется узлом конвейера для получения сообщений от вышестоящих узлов конвейера.
        puller.connect("tcp://localhost:3" + std::to_string(getpid()));
        zmq::context_t context2(1);
        zmq::socket_t pusher(context2,
                             ZMQ_PUSH); // Сокет типа ZMQ_PUSH используется узлом конвейера для отправки сообщений нижестоящим узлам конвейера.
        pusher.connect("tcp://localhost:3" + std::to_string(getpid() + 1));
        std::thread thr[1];
        thr[0] = std::thread(process_server, std::ref(puller), login);
        thr[0].detach(); // отделяет поток выполнения от объекта thread
        process_terminal(pusher, login);
        thr[0].join(); // блокирует поток до завершения действия
        context1.close();
        context2.close();
        puller.disconnect("tcp://localhost:3" + std::to_string(getpid()));
        pusher.disconnect("tcp://localhost:3" + std::to_string(getpid() + 1));
    }
    context.close();
    socket_for_login.disconnect("tcp://localhost:4042");
    return 0;
}