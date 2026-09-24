#pragma once

#include <memory>

#include <editor/editor_context.hpp>
#include <utils/result.hpp>
#include <osal/iwindow.hpp>
#include <rendering/irenderer.hpp>
#include <osal/ifile_system.hpp>

#include <osal/os_context.hpp>
#include <resources/resources_context.hpp>
#include <node_system/node_system_context.hpp>
#include <rendering/rendering_context.hpp>
#include <resources/asset_manager.hpp>
#include <node_system/scene_factory.hpp>

namespace bird {

class Editor {
 public:
  Result<void> init();
  Result<void> run();
  Result<void> shutdown();

  Result<void> make_context_current();

 private:
  EditorContext context;
  OSContext os_context;
  ResourcesContext resources_context;
  NodeSystemContext node_system_context;
  RenderingContext rendering_context;

  std::unique_ptr<IFileSystem> file_system;
  std::unique_ptr<AssetManager> asset_manager;
  std::unique_ptr<IWindow> window;
  std::unique_ptr<IRenderer> renderer;
  std::unique_ptr<SceneFactory> scene_factory;
};

} // bird