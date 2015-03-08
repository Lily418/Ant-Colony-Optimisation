#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <limits>


using namespace rapidjson;

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

const int nodeCount = 4;
float graph[nodeCount][nodeCount];

template <size_t nc>
void initGraph(float (&graph)[nc][nc])
{
    for(int i = 0; i < nc; i++){
        for(int j = 0; j < nc; j++){
            graph[i][j] = 1;

            if(i == 0 && j == 3){
                graph[i][j] = std::numeric_limits<float>::infinity();
            }
        }
    }
}

void writeGraphToBuffer(Writer<StringBuffer> &writer){
    writer.StartObject();
    writer.String("graph");
    writer.StartArray();
    for(int i = 0; i < nodeCount; i++){
        writer.StartArray();
        for(int j = 0; j < nodeCount; j++){
            writer.Double(graph[i][j]);
        }
        writer.EndArray();

    }
    writer.EndArray();
    writer.EndObject();
}

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    StringBuffer reply;
    Writer<StringBuffer> writer(reply);
    writeGraphToBuffer(writer);

    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    try {
        s->send(hdl, reply.GetString(), msg->get_opcode());
    } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Echo failed because: " << e
                  << "(" << e.message() << ")" << std::endl;
    }
}

int main() {
    initGraph(graph);

    // Create a server endpoint
    server echo_server;

    try {
        // Set logging settings
        echo_server.set_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        echo_server.init_asio();

        // Register our message handler
        echo_server.set_message_handler(bind(&on_message,&echo_server,::_1,::_2));

        // Listen on port 9002
        echo_server.listen(9002);

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        echo_server.run();

        std::cout << "Hello Hello" << std::endl;

    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}
