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

	void render_text(std::string text, glm::vec2 pos, glm::uvec2 const& drawable_size);

	//----- game state -----

	//text rendering: (see PlayMode() for initialization)
	FT_Library ft_library;
	FT_Face ft_face;
	hb_font_t* hb_font;
	hb_buffer_t* hb_buffer;

	GLuint VAO, VBO, texture, sampler, vs ,fs, program;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//background music
	// std::shared_ptr< Sound::PlayingSample > bg_loop;

	//game-related strings
	std::string message;

};
