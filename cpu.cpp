#include <limits>
#include <iostream>
#include <random>
#include <array>
#include <math.h>

const unsigned int ANT_COUNT = 500;
const unsigned int MAX_STEPS = 1000;
size_t paths[ANT_COUNT][MAX_STEPS];
std::default_random_engine generator;

const float IMPORTANCE_OF_PHEROMONE = 1;
const float IMPORTANCE_OF_PREVIOUSLY_VISITED = 500;
const float PHEROMONE_EVAPORATION = 0.5;
const float TOTAL_PHEROMONE_FOR_TRAIL = 10000;

void initialize(int startNodeId)
{
    for (unsigned int i = 0; i < ANT_COUNT; i++) {
        paths[i][0] = startNodeId;
    }
}

/* Scale values in an array so the sum of all values is 1. */
template <size_t length>
void normaliseProbabilityDistribution(std::array<float, length>& distribution)
{
    float sum = 0;

    // sum the values
    for (const float& n : distribution) {
        sum += n;
    }

    // adjust them
    for (float& n : distribution) {
        n /= sum;
    }
}

template <size_t nodeCount>
void update(float (&weights)[nodeCount][nodeCount], float (&graph)[nodeCount][nodeCount], size_t goal)
{
    //Ants randomly wander for MAX_STEPS
    for (unsigned int step = 0; step < MAX_STEPS - 1; step++) {
        for (unsigned int antIndex = 0; antIndex < ANT_COUNT; antIndex++) {
            size_t position = paths[antIndex][step];
            if (position != goal) {
                std::array<float, nodeCount> probabilityOfAntMovingToNode;
                for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
                    if (nodeIndex == position
                            || graph[position][nodeIndex] == std::numeric_limits<float>::infinity()) {
                        probabilityOfAntMovingToNode[nodeIndex] = 0;
                    }
                    else {
                        probabilityOfAntMovingToNode[nodeIndex] = weights[position][nodeIndex] * IMPORTANCE_OF_PHEROMONE;

                        //If ant has visited location previously then reduce the probability
                        for (unsigned int i = 0; i < step; i++ ) {
                            if (paths[antIndex][i] == nodeIndex) {
                                probabilityOfAntMovingToNode[nodeIndex] *= (1.0 / exp(i)) * 0.5 * IMPORTANCE_OF_PREVIOUSLY_VISITED;
                            }
                            else {
                                probabilityOfAntMovingToNode[nodeIndex] *= 1 * IMPORTANCE_OF_PREVIOUSLY_VISITED;
                            }
                        }
                    }
                }

                // make probabilities sum to 1
                normaliseProbabilityDistribution(probabilityOfAntMovingToNode);

                //Select based on the probabilties a new location
                std::discrete_distribution<size_t> distribution(probabilityOfAntMovingToNode.begin(), probabilityOfAntMovingToNode.end());
                position = distribution(generator);
            }
            paths[antIndex][step + 1] = position;
        }
    }

    //Pheromone evaporation
    for (size_t weightIndex = 0; weightIndex < nodeCount; weightIndex++) {
        for (size_t weightIndex2 = weightIndex + 1; weightIndex2 < nodeCount; weightIndex2++) {
            weights[weightIndex][weightIndex2] *= PHEROMONE_EVAPORATION;
            //Keep weights matrix consistent
            weights[weightIndex2][weightIndex]  = weights[weightIndex][weightIndex2];
        }
    }

    unsigned int winning_ants = 0;
    //Pheromone update
    for (unsigned int pathIndex = 0; pathIndex < ANT_COUNT; pathIndex++) {

        // skip paths that did not reach the goal
        if (paths[pathIndex][MAX_STEPS - 1] != goal) {
            continue;
        }

        winning_ants++;
        unsigned int length_of_path = MAX_STEPS;
        //Paths tracks the position of the ant at each step, once an ant has reached the goal it will stay there
        //while other ants continue to move
        while (paths[pathIndex][length_of_path - 1] == goal) {
            length_of_path--;
        }

        //This will cause race conditions on GPU if 2 ants used the same edge at the same time
        for (unsigned int i = 0; i < MAX_STEPS; i++) {
            if (paths[pathIndex][i] != goal) {
                weights[paths[pathIndex][i]][paths[pathIndex][i + 1]] += TOTAL_PHEROMONE_FOR_TRAIL / length_of_path;
                weights[paths[pathIndex][i + 1]][paths[pathIndex][i]] = weights[paths[pathIndex][i]][paths[pathIndex][i + 1]];
            }
        }
    }

    std::cout << winning_ants << std::endl;
}
