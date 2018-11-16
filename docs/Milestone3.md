# Milestone 3
[Home](./index.md)

## Goals

The purpose of this milestone was to overhaul our robot's top-level decision making while navigating through a sample maze.  While in the past our robot was capable of right-hand wall following and other simple path-following algorithms, we focused on efficient maze exploration this week by implementing a depth-first search algorithm.
    
## Materials    

No additional materials were implemented for this Milestone.

## DFS

Depth-first search is an algorithm used to traverse the nodes of tree and graph structures by 'visiting' each node. In a depth-first search (DFS, henceforth), the next node to visit is selected by determining which adjacent nodes have not been visited yet, selecting one node, and travelling there, repeating the process on arrival.  In the below graphic, the nodes in the graph are numbered in the order that a DFS will visit them:

<img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/1f/Depth-first-tree.svg/1200px-Depth-first-tree.svg.png" alt="DFS Graphic" width="600" height="385">

