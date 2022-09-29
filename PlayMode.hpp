#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>
#include <hb.h>
#include <hb-ft.h>
#include <freetype/freetype.h>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	void render_text(std::string text, glm::vec2 pos, glm::uvec2 const& drawable_size, glm::vec3 color);

	void add_message(std::string msg, int wait = 0, glm::vec3 color = glm::vec3(0.0f));
	void update_message(std::string msg, int wait = 0);
	void output_text(glm::uvec2 const &drawable_size);
	void take_input();

	//----- game state -----
	enum {
		WAITING, // waiting while texts play out
		PLAYING, // making a choice
	} game_state;

	//text rendering: (see PlayMode() for initialization)
	FT_Library ft_library;
	FT_Face ft_face;
	hb_font_t* hb_font;
	hb_buffer_t* hb_buffer;

	GLuint VAO, VBO, texture, sampler, vs ,fs, program;

	//game-related
	glm::vec3 default_color = glm::vec3(0.5f, 0.8f, 0.2f);
	glm::vec3 choice_color = glm::vec3(0.6f, 0.4f, 0.6f);
	glm::vec3 incorrect_color = glm::vec3(0.8f, 0.2f, 0.2f);
	glm::vec3 win_color = glm::vec3(1.0f);

	uint8_t MAX_MSG_SIZE = 7;
	std::vector<std::string> messages;
	std::vector<glm::vec3> colors;
	std::vector<std::tuple<std::string, int>> user_choices;
	uint8_t hovering_text = 0;

	std::vector<std::tuple<int, std::string, int, glm::vec3>> msg_stack; // action x msg x wait
	std::vector<std::vector<std::tuple<std::string, int>>> choice_stack;
	std::vector<int> task_stack;

	int task_index = -1;
	
};
