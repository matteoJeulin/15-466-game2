#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct GameMode : Mode
{
	GameMode();
	virtual ~GameMode();

	// functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	void randomise_position(Scene::Transform *transform);

	// input tracking:
	struct Button
	{
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	// local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	// References to important objects in the scene
	Scene::Transform *fish = nullptr;
	Scene::Transform *body = nullptr;
	Scene::Transform *back_fin = nullptr;
	Scene::Transform *shark = nullptr;
	Scene::Transform *deadlyTooth = nullptr;

	Scene::Transform *sky = nullptr;
	Scene::Transform *ground = nullptr;
	Scene::Transform *frontWall = nullptr;
	Scene::Transform *backWall = nullptr;
	Scene::Transform *leftWall = nullptr;
	Scene::Transform *rightWall = nullptr;
	std::vector<Scene::Transform *> pointBalls = {};

	glm::quat body_base_rotation;
	glm::quat back_fin_base_rotation;

	bool lost = false;

	// Acceleration and max speed of the fish, accounting for the smaller parent node of the mesh
	const float playerAcceleration = 5.0f * 100.0f;

	// Player and shark speeds have different units because of their scale in blender
	float playerMaxSpeed = 30.0f * 100.0f;
	float sharkMaxSpeed = 20.0f;

	glm::vec3 playerSpeed = glm::vec3(0.0f);

	float wobble = 0.0f;

	// Default game boundaries
	float max_x = 200;
	float min_x = -200;
	float max_y = 100;
	float min_y = -100;
	float max_z = 100;
	float min_z = -100;

	uint32_t score = 0;

	// Screen text
	std::string screen_text = std::to_string(score);

	// camera:
	Scene::Camera *camera = nullptr;
};
