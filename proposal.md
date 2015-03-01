# Using Ant Colony Optimisation to solve Search problems

*George Brighton & Joel Hoskin*

## Introduction

Ant Colony Optimisation (ACO) is a probabilistic technique for finding optimal paths, based on the behaviour of ants searching for food<sup>1</sup>. We would like to investigate the effectiveness of this algorithm at solving search problems, including route-finding and the travelling salesman.

## Search Problems

Search aims to find the most efficient path between any two nodes in a graph. Similarly, the objective of the travelling salesman problem is to find the shortest path that visits *all* nodes once. ACO is relevant to both problems as it can be modelled using a graph. Our solution would use undirected graphs with edges detailing the distances between nodes, the objective being to minimise the total distance travelled, either between two nodes (search), or when visiting all of them (travelling salesman).

Breath First Search is a deterministic algorithm which is guaranteed to find the optimal solution but it has a computational complexity of O(b^d) where b is the branching factor and d is the depth of the solution. This becomes intractable for large problems. Techniques like A* optimize this using heuristics to intelligently guess which parts of the graph to search first but finding the optimal solution remains computationally difficult.

Although Ant Colony Optimization can find optimal solutions in finite time it's difficult to estimate the speed at which it would find the optimal solution. Instead it's best suited to situations where a close to optimal solution is acceptable as it can find reasonable approximates faster than Breath First Search would find an exact solution. It can also applied to problems where the graph changes in real time where as Breath First Search would need to start again.

We have chosen this problem as we hope to be able to visualize the graph and the algorithm as it runs. As well as being able to quantify how close to optimal our output is. 

<sup>1</sup> <cite>Wolfram MathWorld, [Ant Colony Algorithm](http://mathworld.wolfram.com/AntColonyAlgorithm.html), accessed 01/03/15</cite>
