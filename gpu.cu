#include <iostream>
#include <limits>
#include <cstdlib>
#include <iostream>


#include "gpu.hpp"
#include "cutil_inline.h"
#include "curand.h"
#include "const.h"

const unsigned int ANT_COUNT = 512;
const unsigned int MAX_STEPS = 100;

const float IMPORTANCE_OF_PHEROMONE = 5;
const float PHEROMONE_EVAPORATION = 0.2;
const float TOTAL_PHEROMONE_FOR_TRAIL = 100;

size_t paths_gpu[ANT_COUNT][MAX_STEPS];

void generateRandomNumbers(float *a, size_t amount){
    for(int i = 0; i < amount; i++){
        a[i] = (float)rand()/(float)(RAND_MAX);
    }
}


__device__
int selectFromDistribution(float *probabilityDistribution, float r){
    float sum = 0;
    for(int i = 0; i < NODE_COUNT; i++){
        sum += probabilityDistribution[i];
        if(r < sum){
            return i;
        }
    }

    return 0;
}

//randomNumbers should be an array of size MAX_STEPS * ANT_COUNT populated using generateRandomNumbers
__global__
void update_ant_positions(size_t goal, size_t *paths, float *weights, float *graph, float *randomNumbers, size_t start)
{
    const size_t antIndex = blockIdx.x * blockDim.x + threadIdx.x;
    paths[antIndex * MAX_STEPS] = start;
    for(int step = 0; step < MAX_STEPS - 1; step++){
        int currentPathIndex = (antIndex * MAX_STEPS) + step;
        int movementLocation = paths[currentPathIndex];
        if(paths[currentPathIndex] != goal){
            float probabilityOfAntMovingToNode[NODE_COUNT];
            for(int nodeIndex = 0; nodeIndex < NODE_COUNT; nodeIndex++){
                probabilityOfAntMovingToNode[nodeIndex] = weights[paths[currentPathIndex] * NODE_COUNT + nodeIndex] * IMPORTANCE_OF_PHEROMONE;
                //Previously visited causes divergence which causes timeout due to poor performance
            }

            float sumOfWeights = 0;
            for(int i = 0; i < NODE_COUNT; i++){
                sumOfWeights += probabilityOfAntMovingToNode[i];
            }

            for(int i = 0; i < NODE_COUNT; i++){
                probabilityOfAntMovingToNode[i] /= sumOfWeights;
            }

            movementLocation = selectFromDistribution(probabilityOfAntMovingToNode, randomNumbers[(step * ANT_COUNT) + antIndex]);

        }
        paths[currentPathIndex + 1] = movementLocation;
    }

}

//BLOCK_SIZE * GRID_SIZE should equal nodeCount squared (Size of weight array)
__global__
void pheromone_evaporation(float *weights)
{
    const size_t offset = blockIdx.x * blockDim.x + threadIdx.x;
    weights[offset] *= PHEROMONE_EVAPORATION;
}

//BLOCK_SIZE * GRID_SIZE should equal ANT_COUNT
__global__
void path_lengths(size_t goal, size_t *paths, size_t *path_lengths)
{
    const size_t antIndex = blockIdx.x * blockDim.x + threadIdx.x;
    path_lengths[antIndex] = 0;
    for (unsigned int j = 0; j < MAX_STEPS; j++) {
        if (paths[antIndex * MAX_STEPS + j] != goal) {
            path_lengths[antIndex]++;
        }
    }
}


void update_gpu(float (*weights)[NODE_COUNT][NODE_COUNT], float (*graph)[NODE_COUNT][NODE_COUNT], size_t goal, size_t nc, size_t start){
    int devID;
    cudaDeviceProp props;

    cutilSafeCall(cudaGetDevice(&devID));
    cutilSafeCall(cudaGetDeviceProperties(&props, devID));

    // calculate the storage requirements of each array
    const size_t paths_length = ANT_COUNT * MAX_STEPS * sizeof(size_t),
    weights_length = nc * nc * sizeof(float),
    graph_length = nc * nc * sizeof(float),
    path_lengths_length = ANT_COUNT * sizeof(size_t),
    random_numbers_length = ANT_COUNT * MAX_STEPS * sizeof(float);

    std::cout << "Path length: " << paths_length << " bytes" << std::endl
    << "Weights length: " << weights_length << " bytes" << std::endl
    << "Graph length: " << graph_length << " bytes" << std::endl
    << "Path lengths length: " << path_lengths_length << " bytes" << std::endl;

    // allocate memory on device
    float *d_weights, *d_graph, *d_random_numbers;
    size_t *d_paths, *d_path_lengths;
    cutilSafeCall(cudaMalloc(&d_paths, paths_length));
    cutilSafeCall(cudaMalloc(&d_weights, weights_length));
    cutilSafeCall(cudaMalloc(&d_graph, graph_length));
    cutilSafeCall(cudaMalloc(&d_path_lengths, path_lengths_length));
    cutilSafeCall(cudaMalloc(&d_random_numbers, random_numbers_length));


    // copy data to device
    cutilSafeCall(cudaMemcpy(d_paths, &paths_gpu, paths_length, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaMemcpy(d_weights, &(*weights), weights_length, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaMemcpy(d_graph, &(*graph), graph_length, cudaMemcpyHostToDevice));


    float h_random_numbers[random_numbers_length];
    generateRandomNumbers(h_random_numbers, random_numbers_length);
    cutilSafeCall(cudaMemcpy(d_random_numbers, &h_random_numbers, random_numbers_length, cudaMemcpyHostToDevice));

    // 0-initialise the path lengths array on the device
    cutilSafeCall(cudaMemset(d_path_lengths, 0, path_lengths_length));

    // calculate dimensions
    unsigned int block_size = props.maxThreadsPerBlock / 4,
    grid_size = ANT_COUNT / block_size;

    update_ant_positions<<<grid_size, block_size>>>(goal, d_paths, d_weights, d_graph, d_random_numbers, start);

    grid_size = (nc * nc) / block_size;
    pheromone_evaporation<<<grid_size, block_size>>>(d_weights);

    grid_size = ANT_COUNT / block_size;
    path_lengths<<<grid_size, block_size>>>(goal, d_paths, d_path_lengths);

    // copy results back from the device
    cutilSafeCall(cudaMemcpy(&(*weights), d_weights, weights_length, cudaMemcpyDeviceToHost));
    cutilSafeCall(cudaMemcpy(&paths_gpu, d_paths, paths_length, cudaMemcpyDeviceToHost));



    //cutilSafeCall(cudaMemcpy(&graph, d_graph, graph_length, cudaMemcpyDeviceToHost));

    size_t h_path_lengths[ANT_COUNT];
    cutilSafeCall(cudaMemcpy(h_path_lengths, d_path_lengths, path_lengths_length, cudaMemcpyDeviceToHost));

    std::cout << std::endl;

    int winning_ants = 0;
    for(int pathIndex = 0; pathIndex < ANT_COUNT; pathIndex++){
        //If reached the goal
        if(h_path_lengths[pathIndex] < MAX_STEPS){
            winning_ants++;

            for(int i = 0; i < MAX_STEPS - 1; i++){
                if(paths_gpu[pathIndex][i] != goal){
                    std::cout << paths_gpu[pathIndex][i + 1] << " ";

                    (*weights)[paths_gpu[pathIndex][i]][paths_gpu[pathIndex][i + 1]] += TOTAL_PHEROMONE_FOR_TRAIL / h_path_lengths[pathIndex];
                    (*weights)[paths_gpu[pathIndex][i + 1]][paths_gpu[pathIndex][i]] += TOTAL_PHEROMONE_FOR_TRAIL / h_path_lengths[pathIndex];
                }
            }

            std::cout << std::endl;

        }
    }

    std::cout << std::endl;

    std::cout << "Winning Ants" << winning_ants << std::endl;

// free device memory
cutilSafeCall(cudaFree(d_paths));
cutilSafeCall(cudaFree(d_weights));
cutilSafeCall(cudaFree(d_graph));
cutilSafeCall(cudaFree(d_path_lengths));
cutilSafeCall(cudaFree(d_random_numbers));

// free host memory
//delete[] h_path_lengths;

// clean up all resources in this process associated with the device
cudaDeviceReset();
}
