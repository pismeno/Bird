# Node System
## General idea
- The node system is a powerful tool for creating complex behaviors and interactions in the Bird Engine. 
- It allows developers to work in OOP style, while using Data-Oriented Design (DOD) principles under the hood.
- The node system is built on top of ECS-like architecture.

## Architecture
- Every node is instance of the same class `Node`, which is essentially just a `NodeID` (see `node_types.hpp`) wrapper, that holds information about who is it's parent and children.
- The `Scene` defines, what managers will be used to manage the nodes.
- The managers are responsible for the actual node behavior.

## Managers
- Every manager inherits from the `INodeManager` interface.
- Managers keep NodeIDs and hold their own information about the nodes.
- They use preferably contiguous memory to store the data, and use the NodeID as an index to access the data.

## Nodes
- For now, all Nodes are instantiated using the factory methods in `Scene` like `Scene::create_node(...)` or `Scene::create_node_2d(...)`.
- Those methods basically just create a new Node, bind the ids to the new node (like parent_id), register it for the needed systems and return it.