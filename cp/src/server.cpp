#include <iostream>
#include <map>
#include <zmq.hpp>
#include <vector>
#include <cstring>
#include <memory>
#include <thread>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

std::map<std::string, std::shared_ptr<zmq::socket_t>> ports;
std::map<std::string, bool> logged_in;
std::map<std::string, std::vector<std::string>> logged_group;

zmq::context_t context1(1);

void history_save(std::string login_sender, std::string login_accepter, std::string message,
                  std::map<std::string, std::pair<std::string, std::string>> &history_of_messages) {
    std::cout << message + " to " + login_accepter << std::endl;
    std::pair<std::string, std::string> accepter_message(login_accepter,
                                                         history_of_messages[login_sender].second.append(
                                                                 "\n" + message));
    history_of_messages.insert(std::make_pair(login_sender, accepter_message));
}

void history_group_save(std::string login_sender, std::string group, std::string message,
                        std::map<std::string, std::pair<std::string, std::string>> &history_of_messages) {
    std::cout << message + " in " + group + " group" << std::endl;
    std::pair<std::string, std::string> accepter_message(group,
                                                         history_of_messages[login_sender].second.append(
                                                                 "\n" + message));
    history_of_messages.insert(std::make_pair(login_sender, accepter_message));
}

void send_message(std::string message_string, zmq::socket_t &socket) { // отправляем сообщение в сокет
    zmq::message_t message_back(message_string.size());
    memcpy(message_back.data(), message_string.c_str(), message_string.size());
    if (!socket.send(message_back)) {
        std::cout << "Error" << std::endl;
    }
}

std::string receive_message(zmq::socket_t &socket) { // получаем сообщение из сокета
    zmq::message_t message_main;
    socket.recv(&message_main);
    std::string answer(static_cast<char *>(message_main.data()), message_main.size());
    return answer;
}

void process_client(int id, std::map<std::string, std::pair<std::string, std::string>> &history_of_messages,
                    std::string nickname) {
    zmq::context_t context2(1);
    zmq::socket_t puller(context2, ZMQ_PULL);
    puller.bind("tcp://*:3" + std::to_string(id + 1));
    while (true) {
        std::string command = "";
        std::string client_mes = receive_message(puller);
        for (char i: client_mes) {
            if (i != ' ') {
                command += i;
            } else {
                break;
            }
        }
        int i;
        if (command == "send") {
            std::string recipient = "";
            for (i = 5; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    recipient += client_mes[i];
                } else {
                    break;
                }
            }
            if (logged_in[recipient]) {
                std::string message;
                ++i;
                for (i; i < client_mes.size(); ++i) {
                    message += client_mes[i];
                }
                send_message(client_mes, *ports[recipient]);
                history_save(nickname, recipient, message, history_of_messages);
            } else {
                ++i;
                std::string sender = "";
                for (i; i < client_mes.size(); ++i) {
                    if (client_mes[i] != ' ') {
                        sender += client_mes[i];
                    } else {
                        break;
                    }
                }
                send_message("no client", *ports[sender]);
            }
        } else if (command == "gs") {
            std::string group = "";
            for (i = 3; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    group += client_mes[i];
                } else {
                    break;
                }
            }
            if (!(logged_group[group].empty())) {
                std::string message;
                ++i;
                for (i; i < client_mes.size(); ++i) {
                    message += client_mes[i];
                }
                for (auto user: logged_group[group]) {
                    send_message(client_mes, *ports[user]);
                }
                history_group_save(nickname, group, message, history_of_messages);
            } else {
                ++i;
                std::string sender = "";
                for (i; i < client_mes.size(); ++i) {
                    if (client_mes[i] != ' ') {
                        sender += client_mes[i];
                    } else {
                        break;
                    }
                }
                send_message("no group", *ports[sender]);
            }
        } else if (command == "addgroup") {
            std::string group = "";
            for (i = 9; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    group += client_mes[i];
                } else {
                    break;
                }
            }
            std::cout << group << std::endl;
            i++;
            std::string sender = "";
            for (; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    sender += client_mes[i];
                } else {
                    break;
                }
            }
            if (!(logged_group[group].empty()))
                for (auto user: logged_group[group]) {
                    if (logged_group[group].back() == user) logged_group[group].push_back(sender);
                    send_message("you are already a member " + group + " group", *ports[sender]);
                }
            else {
                logged_group[group].push_back(sender);
                std::cout << "User " + sender + " joined to " << group + " group" << std::endl;
                std::cout << "Users in " + group + ": ";
                for (auto user: logged_group[group]) {
                    std::cout << user + " ";
                }
                send_message("group - " + group + ", added to your group list", *ports[sender]);
            }
        } else if (command == "leavegroup") {
            std::string group = "";
            for (i = 11; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    group += client_mes[i];
                } else {
                    break;
                }
            }
            i++;
            std::string sender = "";
            for (i = 10; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    sender += client_mes[i];
                } else {
                    break;
                }
            }
            i = 0;
            for (auto user: logged_group[group]) {
                if (sender != user) i++;
                else {
                    logged_group[group].erase(logged_group[group].begin() + i);
                    break;
                }
            }
            logged_group[group].erase(logged_group[group].begin() + i);
            send_message("leaved " + group, *ports[sender]);
        } else if (command == "grouplist") {
            std::string sender = "";
            for (i = 10; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    sender += client_mes[i];
                } else {
                    break;
                }
            }
            std::string answer;
            for (auto group: logged_group) {
                for (auto user: group.second) {
                    if (user == sender) answer += group.first + " ";
                }
            }
            send_message("your groups: " + answer, *ports[sender]);
        } else if (command == "exit") {
            std::string sender = "";
            for (i = 5; i < client_mes.size(); ++i) {
                if (client_mes[i] != ' ') {
                    sender += client_mes[i];
                } else {
                    break;
                }
            }
            send_message("exit", *ports[sender]);
            logged_in[sender] = false;
        }
    }
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket_for_login(context, ZMQ_REP);

    socket_for_login.bind("tcp://*:4042"); // принимаем соединение через сокет

    std::map<std::string, std::pair<std::string, std::string>> history_of_messages;

    while (true) {
        std::string recieved_message = receive_message(socket_for_login);
        std::string id_s = "";
        int i;
        for (i = 0; i < recieved_message.size(); ++i) {
            if (recieved_message[i] != ' ') {
                id_s += recieved_message[i];
            } else {
                break;
            }
        }
        int id = std::stoi(id_s);
        std::string nickname;
        ++i;
        for (i; i < recieved_message.size(); ++i) {
            if (recieved_message[i] != ' ') {
                nickname += recieved_message[i];
            } else {
                break;
            }
        }
        std::string group;
        ++i;
        for (i; i < recieved_message.size(); ++i) {
            if (recieved_message[i] != ' ') {
                group += recieved_message[i];
            } else {
                break;
            }
        }
        if (logged_in[nickname]) {
            std::cout << "This user already logged in..." << std::endl;
            send_message("0", socket_for_login);
        } else {
            logged_in[nickname] = true;
            logged_group[group].push_back(nickname);
            std::cout << "User " << nickname << " logged in " + group + " with id " << id << std::endl;
            std::cout << "Users in " + group + ": ";
            for (auto user: logged_group[group]) {
                std::cout << user + " ";
            }
            std::cout << std::endl;
            send_message("1", socket_for_login);
            std::shared_ptr<zmq::socket_t> socket_client = std::make_shared<zmq::socket_t>(context1, ZMQ_PUSH);
            socket_client->bind("tcp://*:3" + id_s);
            ports[nickname] = socket_client;
            std::thread worker = std::thread(std::ref(process_client), id, std::ref(history_of_messages),
                                             nickname); //ref - оборачивает ссылку
            worker.detach(); // отделяет поток выполнения от объекта thread
        }
    }
}

#pragma clang diagnostic pop