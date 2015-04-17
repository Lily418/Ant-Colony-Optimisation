#include <unordered_set>
#include <limits>
#include <iostream>
#include <vector>


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
