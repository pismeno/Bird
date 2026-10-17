# Asset Meta Files
Asset meta files are editor-managed files that contain information about assets.
## What do they contain?
- All the Processed Assets' UUIDs and their types
## Structure
Example structure:
```json
{
  "processed_assets" : [
    {
      "uuid" : 123456789,
      "type" : "texture"
    },
    {
      "uuid" : 987654321,
      "type" : "vertex_shader"
    }
  ]
}
```