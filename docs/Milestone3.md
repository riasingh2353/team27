# Milestone 3
[Home](./index.md)

## Goals

The purpose of this milestone was to overhaul our robot's top-level decision making while navigating through a sample maze.  While in the past our robot was capable of right-hand wall following and other simple path-following algorithms, we focused on efficient maze exploration this week by implementing a depth-first search algorithm.
    
## Materials    

No additional materials were implemented for this Milestone.

## DFS

Depth-first search is an algorithm used to traverse the nodes of tree and graph structures by 'visiting' each node. In a depth-first search (DFS, henceforth), the next node to visit is selected by determining which adjacent nodes have not been visited yet, selecting one node, and travelling there, repeating the process on arrival.  In the below graphic, the nodes in the graph are numbered in the order that a DFS will visit them:

<img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/1f/Depth-first-tree.svg/1200px-Depth-first-tree.svg.png" alt="DFS Graphic" width="600" height="385">

On the field, our robot appears to work it's way into a nook of the maze before backtracking and exploring an adjacent nook, and repeating this process until the entire maze has been traversed.  We implement this onboard in memory, as our robot keeps track of all unexplored nodes on the map that, at one point, had been adjacent to the robot.  When the robot reaches a new, undiscovered intersection, it will push the locations of three adjacent nodes into a stack, so long as there isn't a wall present.  To choose where to go next, the robot pops a location from the stack and advances.  As a result, when the robot reaches a node that is surrounded by walls in all directions except the one from which it approached, it will backtrack and travel to a previously-pushed location.

When we selected DFS to implement for this milestone, we compared  with similar graph-traversal algorithms, including Breadth-First Search (BFS), Dijkstra's algorithm, and A*.  
