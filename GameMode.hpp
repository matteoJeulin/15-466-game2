#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct GameMode : Mode {
	GameMode();
	virtual ~GameMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *fish = nullptr;
	Scene::Transform *body = nullptr;
	Scene::Transform *back_fin = nullptr;
	Scene::Transform *shark = nullptr;

	glm::quat body_base_rotation;
	glm::quat back_fin_base_rotation;

	// Acceleration and max speed of the fish, accounting for the smaller parent node of the mesh
	const float playerAcceleration = 5.0f * 100.0f;
	float maxSpeed = 30.0f * 100.0f;

	glm::vec3 playerSpeed = glm::vec3(0.0f);

	float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
