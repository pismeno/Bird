# Bird Engine
Bird Engine is a Specialized 2d game engine. 

## Features
### ECS System
- GameObjects, Components
- prefabs, possibility to use them and override specific traits on them
- 
### Embedded AngelScript
- possibility to use it as a script component
- 
### Serialization / Deserialization
- saving and loading projects using json files
- possibility to export scenes into lvl files
- build in support for saving loading game states (in-game game save files)
- uses FlatBuffers
### Plugins
- 3rd party plugins
- specialized plugins for specific tasks, like datalog, physics

## Used libraries
- FlatBuffers
- AngelScript
- Dear ImGui
- spdlog
- nlohmann/json
- GLM
- OpenGL
- GLFW