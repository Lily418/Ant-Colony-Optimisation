#include <limits>
#include <iostream>
#include <random>
#include <array>
#include <math.h>

const int ANT_COUNT = 500;
const int MAX_STEPS = 1000;
int paths[ANT_COUNT][MAX_STEPS];
std::default_random_engine generator;

const float IMPORTANCE_OF_PHEROMONE = 1;
const float IMPORTANCE_OF_PREVIOUSLY_VISITED = 500;
const float PHEROMONE_EVAPORATION = 0.5;
const float TOTAL_PHEROMONE_FOR_TRAIL = 10000;

void initialize(int startNodeId)
{
    for (int i = 0; i < ANT_COUNT; i++) {
        paths[i][0] = startNodeId;
    }
}

template <size_t nc>
void update(float (&weights)[nc][nc], float (&graph)[nc][nc], int goal)
{
    //Ants randomly wander for MAX_STEPS
    for (int step = 0; step < MAX_STEPS - 1; step++) {
        for (int antIndex = 0; antIndex < ANT_COUNT; antIndex++) {
            int movementLocation = paths[antIndex][step];
            if (paths[antIndex][step] != goal) {
                std::array<float, nc> probabilityOfAntMovingToNode;
                for (int nodeIndex = 0; nodeIndex < nc; nodeIndex++) {
                    if (nodeIndex == paths[antIndex][step]
                            || graph[paths[antIndex][step]][nodeIndex] == std::numeric_limits<float>::infinity()) {
                        probabilityOfAntMovingToNode[nodeIndex] = 0;
                    }
                    else {
                        probabilityOfAntMovingToNode[nodeIndex] = weights[paths[antIndex][step]][nodeIndex] * IMPORTANCE_OF_PHEROMONE;

                        //If ant has visited location previously then reduce the probability
                        for (int i = 0; i < step; i++ ) {
                            if (paths[antIndex][i] == nodeIndex) {
                                probabilityOfAntMovingToNode[nodeIndex] *= (1.0 / exp(i)) * 0.5 * IMPORTANCE_OF_PREVIOUSLY_VISITED;
                            }
                            else {
                                probabilityOfAntMovingToNode[nodeIndex] *= 1 * IMPORTANCE_OF_PREVIOUSLY_VISITED;
                            }
                        }
                    }
                }

                //Make probabilities sum to 1
                float sumOfWeights = 0;
                for (int i = 0; i < nc; i++) {
                    sumOfWeights += probabilityOfAntMovingToNode[i];
                }

                for (int i = 0; i < nc; i++) {
                    probabilityOfAntMovingToNode[i] /= sumOfWeights;
                }

                //Select based on the probabilties a new location
                std::discrete_distribution<int> distribution(probabilityOfAntMovingToNode.begin(), probabilityOfAntMovingToNode.end());
                movementLocation = distribution(generator);
            }
            paths[antIndex][step + 1] = movementLocation;
        }
    }

    //Pheromone evaporation
    for (int weightIndex = 0; weightIndex < nc; weightIndex++) {
        for (int weightIndex2 = weightIndex + 1; weightIndex2 < nc; weightIndex2++) {
            weights[weightIndex][weightIndex2] *= PHEROMONE_EVAPORATION;
            //Keep weights matrix consistent
            weights[weightIndex2][weightIndex]  = weights[weightIndex][weightIndex2];
        }
    }

    int winning_ants = 0;
    //Pheromone update
    for (int pathIndex = 0; pathIndex < ANT_COUNT; pathIndex++) {
        //If the path reached the goal
        if (paths[pathIndex][MAX_STEPS - 1] == goal) {
            winning_ants++;
            int length_of_path = MAX_STEPS;
            //Paths tracks the position of the ant at each step, once an ant has reached the goal it will stay there
            //while other ants continue to move
            while (paths[pathIndex][length_of_path - 1] == goal) {
                length_of_path--;
            }

            //This will cause race conditions on GPU if 2 ants used the same edge at the same time
            for (int i = 0; i < MAX_STEPS; i++) {
                if (paths[pathIndex][i] != goal) {
                    weights[paths[pathIndex][i]][paths[pathIndex][i + 1]] += TOTAL_PHEROMONE_FOR_TRAIL / length_of_path;
                    weights[paths[pathIndex][i + 1]][paths[pathIndex][i]] = weights[paths[pathIndex][i]][paths[pathIndex][i + 1]];
                }
            }
        }
    }

    std::cout << winning_ants << std::endl;
}
