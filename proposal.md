# Using Ant Colony Optimisation to solve Search problems

*George Brighton & Joel Hoskin*

## Introduction

Ant Colony Optimisation (ACO) is a probabilistic technique for finding optimal paths, based on the behaviour of ants searching for food<sup>1</sup>. We would like to investigate the effectiveness of this algorithm at solving search problems, including route-finding and the travelling salesman.

## Search Problems

Search aims to find the most efficient path between any two nodes in a graph. Similarly, the objective of the travelling salesman problem is to find the shortest path that visits *all* nodes once. ACO is relevant to both problems as it can be modelled using a graph. Our solution would use undirected graphs with edges detailing the distances between nodes, the objective being to minimise the total distance travelled, either between two nodes (search), or when visiting all of them (travelling salesman).

## Motivation

There are already many algorithms that can be used to solve search and travelling salesman problems, some optimally, however they tend to be inefficient:

 - Breadth-first search is optimal, but has a time complexity of <code>O(b<sup>d</sup>)</code>, where `b` is the branching factor and `d` is the depth of the solution.
 - The travelling salesman problem can be solved using brute force in <code>O(n!)</code> time, however this makes it impractical for only 20 cities. The Held–Karp algorithm solves the problem in <code>O(n<sup>2</sup>2<sup>n</sup>)</code> time, however this is still intractable for large problems.

Heuristics can be employed to increase the rate at which both problems can be solved, however finding the optimal solution is still computationally difficult. ACO can find optimal solutions in finite time, however it is difficult to estimate the speed at which a solution can be calculated. It is better suited to problems where a close-to-optimal solution is acceptable.

We have chosen this problem as we hope to be able to visualize the graph and the algorithm as it runs. As well as being able to quantify how close to optimal our output is. 

<sup>1</sup> <cite>Wolfram MathWorld, [Ant Colony Algorithm](http://mathworld.wolfram.com/AntColonyAlgorithm.html), accessed 01/03/15</cite>
