#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#define ASIO_STANDALONE

#include <string>
#include <thread>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

// Type alias for the WebSocket++ client using the asio_client config.
typedef websocketpp::client<websocketpp::config::asio_client> ws_client;

class WebSocketClient {
public:
    // Returns the single instance of the client.
    static WebSocketClient& getInstance();

    // Deleted copy constructor and assignment operator to enforce singleton.
    WebSocketClient(const WebSocketClient&) = delete;
    WebSocketClient& operator=(const WebSocketClient&) = delete;

    // Public interface to connect and send messages.
    void connect(const std::string &uri);
    void send(const std::string &message);

    // Destructor
    ~WebSocketClient();

private:
    // Private constructor to enforce singleton.
    WebSocketClient();

    // Internal event handlers.
    void on_open(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);
    void on_close(websocketpp::connection_hdl hdl);
    void on_fail(websocketpp::connection_hdl hdl);

    // Member variables.
    ws_client m_client;
    websocketpp::connection_hdl m_hdl;
    std::thread m_thread;
};

#endif // WEBSOCKETCLIENT_H
