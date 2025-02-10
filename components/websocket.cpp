//
//  websocket.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-07.
//

#include "websocket.h"
#include <chrono>
#include <functional>
#include <thread>

// TLS initialization handler for standalone Asio.
context_ptr on_tls_init(websocketpp::connection_hdl /*hdl*/) {
    // Create a new SSL context with TLS v1.2 client settings.
    context_ptr ctx =
        std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12_client);

    // Set the desired SSL options.
    ctx->set_options(
        asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |
        asio::ssl::context::no_sslv3 | asio::ssl::context::single_dh_use);

    return ctx;
}

// Private constructor: sets up the ASIO transport and event handlers.
WebSocketClient::WebSocketClient() {
    m_client.init_asio();
    m_client.start_perpetual();

    // Bind the TLS initialization handler.
    m_client.set_tls_init_handler(on_tls_init);

    // Bind the event handlers to the class member functions.
    m_client.set_open_handler(
        std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
    m_client.set_message_handler(std::bind(&WebSocketClient::on_message, this,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
    m_client.set_close_handler(
        std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
    m_client.set_fail_handler(
        std::bind(&WebSocketClient::on_fail, this, std::placeholders::_1));
    
    m_client.clear_access_channels(websocketpp::log::alevel::all);

    // Start the ASIO io_service loop in a separate thread.
    // Since start_perpetual() is used, the run() call will continue running
    // even if there is no active connection.
    m_thread = std::thread([this]() { m_client.run(); });
}

// Destructor: stops the perpetual run and joins the thread.
WebSocketClient::~WebSocketClient() {
    m_client.stop_perpetual();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

// Singleton accessor.
WebSocketClient &WebSocketClient::getInstance() {
    static WebSocketClient instance;
    return instance;
}

// Connects to the WebSocket server at the specified URI.
void WebSocketClient::connect(const std::string &uri) {
    // Store the URI so that we can attempt to reconnect later.
    this->m_uri = uri;
    websocketpp::lib::error_code ec;
    auto con = m_client.get_connection(uri, ec);
    if (ec) {
        LogMsg("WebSocket Conn Failed: %s", ec.message().c_str());
        return;
    }
    m_hdl = con->get_handle();
    m_client.connect(con);
}

// Attempts to reconnect using the stored URI.
void WebSocketClient::reconnect() {
    if (m_uri.empty()) {
        LogMsg("No URI stored. Cannot reconnect.");
        return;
    }
    LogMsg("Reconnecting to server");
    websocketpp::lib::error_code ec;
    auto con = m_client.get_connection(m_uri, ec);
    if (ec) {
        LogMsg("WebSocket Reconnect Failed: %s", ec.message().c_str());
        // Schedule another reconnect attempt after a delay.
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            LogMsg("Attempting to reconnect...");
            this->reconnect();
        }).detach();
        return;
    }
    m_hdl = con->get_handle();
    m_client.connect(con);
}

// Sends a text message to the WebSocket server.
void WebSocketClient::send(const std::string &message) {
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        LogMsg("Send Error: %s", ec.message().c_str());
    }
}

// Event handler called when a connection is established.
void WebSocketClient::on_open(websocketpp::connection_hdl hdl) {
    LogMsg("Connection opened.");
}

// Event handler called when a message is received.
void WebSocketClient::on_message(websocketpp::connection_hdl hdl,
                                 ws_client::message_ptr msg) {
    return;
//    LogMsg("Received message: %s", msg->get_payload().c_str());
}

// Event handler called when the connection is closed.
void WebSocketClient::on_close(websocketpp::connection_hdl hdl) {
    LogMsg("Attempting to reconnect...");
    this->reconnect();
}

// Event handler called when the connection fails.
void WebSocketClient::on_fail(websocketpp::connection_hdl hdl) {
    LogMsg("Attempting to reconnect...");
    this->reconnect();
}
