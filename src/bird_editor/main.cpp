#include <iostream>
#include <memory>

#include "../bird_core/rendering/renderer.hpp"


int main() {
	bird::Renderer renderer;
	renderer.create_renderer(bird::RendererType::Metal);
	renderer.init();
	return 0;
}