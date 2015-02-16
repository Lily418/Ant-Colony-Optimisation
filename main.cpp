#include "mongoose/mongoose.h"
#include <string.h>
#include <sys/time.h>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <limits>
#include <unistd.h>

#include "bfs.cpp"
#include "cpu.cpp"
#include "gpu.hpp"
#include "const.h"

using namespace rapidjson;

const int nodeCount = NODE_COUNT;
const float graphDensity = 0.00005;

//const int nodeCount = 20;
//const float graphDensity = 0.05;

float graph[nodeCount][nodeCount];
float weights[nodeCount][nodeCount];

struct goal{
    size_t start;
    size_t end;
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
void initWeights(float (&weights)[nc][nc], float (&graph)[nc][nc])
{
    for(int i = 0; i < nc; i++){
        for(int j = 0; j < nc; j++){
            if(graph[i][j] == 1){
                weights[i][j] = 1;
            } else {
                weights[i][j] = 0;
            }
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

static int send_reply(struct mg_connection *conn) {
    StringBuffer reply;
    Writer<StringBuffer> writer(reply);
    writeGraphToBuffer(writer);

    std::string replyString = reply.GetString();
    const char *c = replyString.c_str();


    mg_websocket_write(conn, 1, c, replyString.length());
    return conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ?
    MG_FALSE : MG_TRUE;

}


static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
    switch (ev) {
        case MG_AUTH: return MG_TRUE;
        case MG_REQUEST: return send_reply(conn);
        default: return MG_FALSE;
    }
}

void ant_colony()
{
    timeval t1, t2;
    double elapsedTime;

    while(true){
        gettimeofday(&t1, NULL);
        update_gpu(&weights, &graph, g.end, NODE_COUNT, g.start);
        //update(weights, graph, g.end);
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
        cout << elapsedTime << " ms.\n";
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
    initWeights(weights, graph);
    struct goal *newGoal = createGoal();
    g.start = newGoal->start;
    g.end   = newGoal->end;

    initialise(g.start);
    std::thread t1(ant_colony);

    // Create a server endpoint
    struct mg_server *server = mg_create_server(NULL, ev_handler);
    mg_set_option(server, "listening_port", "8080");
    printf("Started on port %s\n", mg_get_option(server, "listening_port"));
    for (;;) {
        mg_poll_server(server, 100);
    }

    mg_destroy_server(&server);
    return 0;

}
