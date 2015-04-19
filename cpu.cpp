#include <limits>
#include <iostream>
#include <random>
#include <array>
#include <math.h>

/* The total number of ants in the simulation. */
const unsigned int ANT_COUNT = 500;

/* The maximum number of moves each ant will make. */
const unsigned int MAX_STEPS = 1000;

const float IMPORTANCE_OF_PHEROMONE = 1;
const float IMPORTANCE_OF_PREVIOUSLY_VISITED = 500;
const float PHEROMONE_EVAPORATION = 0.5;
const float TOTAL_PHEROMONE_FOR_TRAIL = 10000;

/* Details the position of each ant at each step.
   Once the ant has reached the goal, it will stay there while other ants not at the goal continue to move. */
size_t paths[ANT_COUNT][MAX_STEPS];

std::default_random_engine generator;

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
    // ants randomly wander for MAX_STEPS
    for (unsigned int step = 0; step < MAX_STEPS - 1; step++) {
        for (unsigned int antIndex = 0; antIndex < ANT_COUNT; antIndex++) {

            // get the position of the ant at the last step
            size_t position = paths[antIndex][step];

            if (position != goal) {

                // the value at each index corresponds to the probability of this ant moving to that node
                std::array<float, nodeCount> probabilityOfAntMovingToNode;

                for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
                    if (nodeIndex == position // the ant is currently at the node
                            || graph[position][nodeIndex] == std::numeric_limits<float>::infinity()) { // the cannot be reached
                        probabilityOfAntMovingToNode[nodeIndex] = 0;
                    }
                    else {
                        // set the probability to the weight between the two nodes (will normalise later)
                        probabilityOfAntMovingToNode[nodeIndex] = weights[position][nodeIndex] * IMPORTANCE_OF_PHEROMONE;

                        // if the ant has previously visited the node, reduce the probability
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

                // select a random new location based on the probabilties
                std::discrete_distribution<size_t> distribution(probabilityOfAntMovingToNode.begin(), probabilityOfAntMovingToNode.end());
                position = distribution(generator);
            }

            // set the next position to this new set
            paths[antIndex][step + 1] = position;
        }
    }

    // perform pheromone evaporation
    for (size_t weightIndex = 0; weightIndex < nodeCount; weightIndex++) {
        for (size_t weightIndex2 = weightIndex + 1; weightIndex2 < nodeCount; weightIndex2++) {
            weights[weightIndex][weightIndex2] *= PHEROMONE_EVAPORATION;
            // keep weights matrix consistent (will be mirrored diagonally)
            weights[weightIndex2][weightIndex]  = weights[weightIndex][weightIndex2];
        }
    }

    // pheromone update
    unsigned int winning_ants = 0;
    for (unsigned int pathIndex = 0; pathIndex < ANT_COUNT; pathIndex++) {

        // skip paths that did not reach the goal
        if (paths[pathIndex][MAX_STEPS - 1] != goal) {
            continue;
        }

        winning_ants++;

        unsigned int length_of_path = MAX_STEPS;
        while (paths[pathIndex][length_of_path - 1] == goal) {
            length_of_path--;
        }

        //This will cause race conditions on GPU if 2 ants used the same edge at the same time
        for (unsigned int i = 0; i < MAX_STEPS; i++) {
            if (paths[pathIndex][i] != goal) {
                weights[paths[pathIndex][i]][paths[pathIndex][i + 1]] += TOTAL_PHEROMONE_FOR_TRAIL / length_of_path;

                // again, keep weights matrix consistent
                weights[paths[pathIndex][i + 1]][paths[pathIndex][i]] = weights[paths[pathIndex][i]][paths[pathIndex][i + 1]];
            }
        }
    }

    std::cout << winning_ants << std::endl;
}
