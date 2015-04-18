#include <unordered_set>
#include <limits>
#include <iostream>
#include <vector>
#include <list>


using namespace std;

template <size_t nc>
unordered_set<int> connected(int root, float (&graph)[nc][nc]){
    unordered_set<int> open_set;
    unordered_set<int> closed_set;

    open_set.insert(root);
    while(open_set.size() > 0){
        //Select and remove an element from the open_set
        int n = *open_set.begin();
        open_set.erase (open_set.begin());
        closed_set.insert(n);

        for(int i = 0; i < nc; i++){
            if(i != n && graph[n][i] != std::numeric_limits<float>::infinity()){
                //node i is directly connected to n
                //if node not already in open or closed set
                if(open_set.find(i) == open_set.end() && closed_set.find(i) == closed_set.end()){
                    open_set.insert(i);
                }
            }
        }
    }

    return closed_set;
}

struct node {
    int id;
    int cost;
};

struct node *createNode(int id, int cost){
    struct node *n = (struct node*)malloc(sizeof(node));
    n->id = id;
    n->cost = cost;
    return n;
}

int inOpenSet(list<struct node*> open_set, int n){
    for (list<struct node*>::iterator it=open_set.begin(); it != open_set.end(); it++){
        if((*it)->id == n){
            return 1;
        }
    }

    return 0;
}

template <size_t nc>
int distance(int n1, int n2, float (&graph)[nc][nc]){
    list<struct node*> open_set;
    unordered_set<int> closed_set;

    open_set.push_back(createNode(n1, 0));
    while(1){
        struct node *n = open_set.front();
        open_set.pop_front();
        closed_set.insert(n->id);

        for(int i = 0; i < nc; i++){
            if(graph[n->id][i] != std::numeric_limits<float>::infinity()){
                if(i == n2){
                    return n->cost + graph[n->id][i];
                }

                if(closed_set.find(i) == closed_set.end() && !inOpenSet(open_set, i)){
                    open_set.push_back(createNode(i, n->cost + 1));
                }
            }
        }
    }

}

template <size_t nc>
void connect(float (&graph)[nc][nc]){
    unordered_set<int> c;
    vector<int> node_values;
    for(int i = 0; i < nc; i++){
        if(c.find(i) == c.end()){
            unordered_set<int> clique = connected(i, graph);
            if(c.size() != 0){
                //Connect clique to connected
                int connect_to = node_values[rand() % node_values.size()];
                graph[connect_to][*clique.begin()] = 1;
                graph[*clique.begin()][connect_to] = 1;
            }

            for (auto it = clique.begin(); it != clique.end(); it++){
                c.insert(*it);
                node_values.push_back(*it);
            }
        }
    }
}


void printSet(unordered_set<int> set){
    for (auto it = set.begin(); it != set.end(); it++){
        cout << " " << *it;
    }

    cout << endl;
}
