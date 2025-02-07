//
//  websocket.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-07.
//

#include "websocket.h"
#include <iostream>
#include <functional>

// Private constructor: sets up the ASIO transport and event handlers.
WebSocketClient::WebSocketClient() {
    m_client.init_asio();
    m_client.start_perpetual();

    // Bind the event handlers to the class member functions.
    m_client.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
    m_client.set_message_handler(std::bind(&WebSocketClient::on_message, this,
                                             std::placeholders::_1, std::placeholders::_2));
    m_client.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
    m_client.set_fail_handler(std::bind(&WebSocketClient::on_fail, this, std::placeholders::_1));
}

// Destructor: stops the perpetual run and joins the thread.
WebSocketClient::~WebSocketClient() {
    m_client.stop_perpetual();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

// Singleton accessor.
WebSocketClient& WebSocketClient::getInstance() {
    static WebSocketClient instance;
    return instance;
}

// Connects to the WebSocket server at the specified URI.
void WebSocketClient::connect(const std::string &uri) {
    websocketpp::lib::error_code ec;
    auto con = m_client.get_connection(uri, ec);
    if (ec) {
        std::cerr << "Connect initialization error: " << ec.message() << std::endl;
        return;
    }
    m_hdl = con->get_handle();
    m_client.connect(con);
    // Run the ASIO io_service loop in a separate thread.
    m_thread = std::thread([this]() { m_client.run(); });
}

// Sends a text message to the WebSocket server.
void WebSocketClient::send(const std::string &message) {
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "Send Error: " << ec.message() << std::endl;
    }
}

// Event handler called when a connection is established.
void WebSocketClient::on_open(websocketpp::connection_hdl hdl) {
    std::cout << "Connection opened." << std::endl;
}

// Event handler called when a message is received.
void WebSocketClient::on_message(websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
    std::cout << "Received message: " << msg->get_payload() << std::endl;
}

// Event handler called when the connection is closed.
void WebSocketClient::on_close(websocketpp::connection_hdl hdl) {
    std::cout << "Connection closed." << std::endl;
}

// Event handler called when the connection fails.
void WebSocketClient::on_fail(websocketpp::connection_hdl hdl) {
    std::cout << "Connection failed." << std::endl;
}
