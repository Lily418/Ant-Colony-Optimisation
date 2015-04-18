#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <thread>
#include <limits>
#include <unistd.h>

#include "bfs.cpp"
#include "cpu.cpp"


using namespace rapidjson;

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

const int nodeCount = 150;
const float graphDensity = 0.015;
//const int nodeCount = 20;
//const float graphDensity = 0.05;

float graph[nodeCount][nodeCount];
float weights[nodeCount][nodeCount];

struct goal{
    unsigned int start;
    unsigned int end;
}g;


template <size_t nc>
void initGraph(float (&graph)[nc][nc])
{
    for(int i = 0; i < nc; i++){
        graph[i][i] = 0;
    }

    for(int i = 0; i < nc; i++){
        for(int j = i + 1; j < nc; j++){
            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if(r < graphDensity){
                graph[j][i] = graph[i][j] = 1;
            } else {
                graph[j][i] = graph[i][j] = std::numeric_limits<float>::infinity();
            }
        }
    }

    connect(graph);
}

template <size_t nc>
void initWeights(float (&weights)[nc][nc])
{
    for(int i = 0; i < nc; i++){
        for(int j = 0; j < nc; j++){
                weights[i][j] = 1;
            }
        }
}

void writeGraphToBuffer(Writer<StringBuffer> &writer){
    writer.StartObject();
    writer.String("goal");
    writer.StartObject();
    writer.String("start");
    writer.Uint(g.start);
    writer.String("end");
    writer.Uint(g.end);
    writer.EndObject();
    writer.String("graph");
    writer.StartArray();
    for(int i = 0; i < nodeCount; i++){
        writer.StartArray();
        for(int j = 0; j < nodeCount; j++){
            writer.StartObject();
            writer.String("cost");
            writer.Double(graph[i][j]);
            writer.String("weight");
            writer.Double(weights[i][j]);
            writer.EndObject();
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

void ant_colony()
{
    while(true){
        update(weights, graph, g.end);
        usleep(2000000);
    }
}



struct goal *createGoal(){
    int maxDistance = 0;
    int n1 = 0;
    int n2 = 0;

    for(int i = 0; i < 100; i++){
        int a = rand() % nodeCount;
        int b = rand() % nodeCount;
        int d = distance(a,b,graph);
        if(d > maxDistance){
            n1 = a;
            n2 = b;
            maxDistance = d;
        }
    }

    struct goal *g = (struct goal*)malloc(sizeof(struct goal));
    g->start = n1;
    g->end   = n2;
    return g;
}


int main() {
    initGraph(graph);
    initWeights(weights);
    struct goal *newGoal = createGoal();
    g.start = newGoal->start;
    g.end   = newGoal->end;

    initialize(g.start);
    std::thread t1(ant_colony);

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
