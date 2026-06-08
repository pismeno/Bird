# Node System
## General idea
- The node system is a powerful tool for creating complex behaviors and interactions in the Bird Engine. 
- It allows developers to work in OOP style, while using Data-Oriented Design (DOD) principles under the hood.
- The node system is built on top of ECS-like architecture.

## Architecture
- Every node is instance of the same class `Node`, which is essentially just a `NodeID` (see `node_type_definitions.hpp`) wrapper, that holds information about who is it's parent and children.
- The `Scene` defines, what managers will be used to manage the nodes.
- The managers are responsible for the actual node behavior.
- `Scene` is created using `SceneFactory::create_scene(std::string scene_type_name)`, scene types can be loaded using `SceneFactory::load_scene_definitions_from(std::string path)`.
- `Scene` also holds node_type_definitions you can create using the factory method listed bellow.

## Managers
- Every manager inherits from the `INodeManager` interface.
- Managers keep NodeIDs and hold their own information about the nodes.
- They use preferably contiguous memory to store the data, and use the NodeID as an index to access the data.

## Nodes
- Nodes are instantiated using the factory method  `Scene::create_node(NodeID parent_id, std::string node_type)`
- Node types can be loaded into `SceneFactory` using `SceneFactory::load_node_definitions_from(std::string path)`