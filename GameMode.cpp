#include "GameMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint fish_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> fish_meshes(LoadTagDefault, []() -> MeshBuffer const *
							 {
	MeshBuffer const *ret = new MeshBuffer(data_path("fish.pnct"));
	fish_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

Load<Scene> fish_scene(LoadTagDefault, []() -> Scene const *
					   { return new Scene(data_path("fish.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name)
										  {
											  Mesh const &mesh = fish_meshes->lookup(mesh_name);

											  scene.drawables.emplace_back(transform);
											  Scene::Drawable &drawable = scene.drawables.back();

											  drawable.pipeline = lit_color_texture_program_pipeline;

											  drawable.pipeline.vao = fish_meshes_for_lit_color_texture_program;
											  drawable.pipeline.type = mesh.type;
											  drawable.pipeline.start = mesh.start;
											  drawable.pipeline.count = mesh.count; }); });

GameMode::GameMode() : scene(*fish_scene)
{
	// get pointers to leg for convenience:
	for (auto &transform : scene.transforms)
	{
		if (transform.name == "Fish")
			fish = &transform;
		else if (transform.name == "Body")
			body = &transform;
		else if (transform.name == "BackFin")
			back_fin = &transform;
		else if (transform.name == "Shark")
			shark = &transform;
	}

	if (fish == nullptr)
		throw std::runtime_error("Fish not found.");
	if (body == nullptr)
		throw std::runtime_error("Fish body not found.");
	if (back_fin == nullptr)
		throw std::runtime_error("Fish back fin not found.");
	if (shark == nullptr)
		throw std::runtime_error("Shark not found.");

	body_base_rotation = body->rotation;
	back_fin_base_rotation = back_fin->rotation;

	// get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

GameMode::~GameMode()
{
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_EVENT_KEY_DOWN)
	{
		if (evt.key.key == SDLK_ESCAPE)
		{
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		}
		else if (evt.key.key == SDLK_A || evt.key.key == SDLK_LEFT)
		{
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_D || evt.key.key == SDLK_RIGHT)
		{
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_W || evt.key.key == SDLK_UP)
		{
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_S || evt.key.key == SDLK_DOWN)
		{
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	}
	else if (evt.type == SDL_EVENT_KEY_UP)
	{
		if (evt.key.key == SDLK_A || evt.key.key == SDLK_LEFT)
		{
			left.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_D || evt.key.key == SDLK_RIGHT)
		{
			right.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_W || evt.key.key == SDLK_UP)
		{
			up.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_S || evt.key.key == SDLK_DOWN)
		{
			down.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	{
		if (SDL_GetWindowRelativeMouseMode(Mode::window) == false)
		{
			SDL_SetWindowRelativeMouseMode(Mode::window, true);
			return true;
		}
	}
	else if (evt.type == SDL_EVENT_MOUSE_MOTION)
	{
		if (SDL_GetWindowRelativeMouseMode(Mode::window) == true)
		{
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				evt.motion.yrel / float(window_size.y));
			// Make sure the rotation does not roll the character by multiplying yaw on one side
			// and pitch on the other (source: https://stackoverflow.com/questions/46738139/prevent-rotation-around-certain-axis-with-quaternion)
			fish->rotation = glm::normalize(
				glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 0.0f, 1.0f)) * fish->rotation * glm::angleAxis(-motion.y * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f)));
			return true;
		}
	}

	return false;
}

void GameMode::update(float elapsed)
{
	// slowly rotates through [0,1):
	wobble += elapsed / 2.0f;
	wobble -= std::floor(wobble);

	// move camera:
	{
		// combine inputs into a move:
		if (left.pressed && !right.pressed)
			playerSpeed.x = std::max(playerSpeed.x - playerAcceleration * elapsed, -maxSpeed);
		if (!left.pressed && right.pressed)
			playerSpeed.x = std::min(playerSpeed.x + playerAcceleration * elapsed, maxSpeed);
		if (down.pressed && !up.pressed)
			playerSpeed.y = std::max(playerSpeed.y - playerAcceleration * elapsed, -maxSpeed);
		if (!down.pressed && up.pressed)
			playerSpeed.y = std::min(playerSpeed.y + playerAcceleration * elapsed, maxSpeed);

		if (!left.pressed && !right.pressed)
		{
			playerSpeed.x -= playerSpeed.x * elapsed;
			playerSpeed.z -= playerSpeed.z * elapsed;
		}
		if (!up.pressed && !down.pressed)
		{
			playerSpeed.y -= playerSpeed.y * elapsed;
		}

		back_fin->rotation = back_fin_base_rotation * glm::angleAxis(
														  (glm::length(playerSpeed) / maxSpeed) * (float(M_PI) / 4) * std::sin(wobble * 2.0f * float(M_PI)),
														  glm::vec3(0.0f, 1.0f, 0.0f));

		body->rotation = body_base_rotation * glm::angleAxis(
												  (-glm::length(playerSpeed) / (maxSpeed)) * (float(M_PI) / 32) * std::sin(wobble * 2.0f * float(M_PI)),
												  glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4x3 frame = fish->make_parent_from_local();
		glm::vec3 frame_forward = -frame[0];
		glm::vec3 frame_right = frame[1];
		glm::vec3 frame_up = -frame[2];

		// y-axis is the forward/backward direction and the x-axis is the right/left direction
		fish->position += playerSpeed.x * frame_right * elapsed + playerSpeed.y * frame_forward * elapsed + playerSpeed.z * frame_up * elapsed;

		// Based on : https://stackoverflow.com/questions/13014973/quaternion-rotate-to
		glm::mat4x3 frame_shark = shark->make_parent_from_local();
		glm::vec3 frame_shark_forward = -frame_shark[0]; // Assuming -X is forward

		glm::vec3 p_c = glm::normalize(fish->position - shark->position);

		glm::vec3 a = glm::cross(frame_shark_forward, p_c);

		if (a != glm::vec3(0.0f) && p_c != glm::vec3(0.0f))
		{
			a = glm::normalize(a);
			float dot = glm::dot(glm::normalize(frame_shark_forward), p_c);
			if (dot < 1 && dot > -1)
			{
				float phi = glm::acos(dot);
				glm::vec3 b = glm::cross(a, glm::normalize(frame_shark_forward));

				if (glm::dot(b, p_c) < 0)
				{
					phi = -phi;
				}

				glm::quat rot = glm::angleAxis(phi, a);
				shark->rotation = rot * shark->rotation;
			}
		}
		shark->position += frame_shark_forward * (maxSpeed / 300) * elapsed;
	}
}

void GameMode::draw(glm::uvec2 const &drawable_size)
{
	// update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	// set up light type and position for lit_color_texture_program:
	//  TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.25f, 0.71f, 0.84f, 1.0f);
	glClearDepth(1.0f); // 1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); // print any errors produced by this setup code

	scene.draw(*camera);

	{ // use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
						glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
						glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
