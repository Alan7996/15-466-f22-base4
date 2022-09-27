#include "PlayMode.hpp"

#include "ColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

Load< Sound::Sample > bg_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("bg.wav"));
});

PlayMode::PlayMode() {
	
	// bit of help from David Lyons on where to get started with this
	// based on https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
	FT_Init_FreeType(&ft_library);
	FT_New_Face(ft_library, data_path("NotoSansMono-Bold.ttf").c_str(), 0, &ft_face);
	FT_Set_Char_Size(ft_face, 36 * 64, 36 * 64, 0, 0);

	hb_font = hb_ft_font_create(ft_face, NULL);
	hb_buffer = hb_buffer_create();

	//start music loop playing:
	//bg_loop = Sound::loop(*bg_sample);
	
	message = "hello world";
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			message = "what";
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			message = "is";
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			message = "up";
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			message = "fam";
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{ //render texts
		glDisable(GL_DEPTH_TEST);
		
		render_text(message, glm::vec2(drawable_size.x / 10.0f, drawable_size.y / 10.0f), drawable_size);
	}
	GL_ERRORS();
}

void PlayMode::render_text(std::string text, glm::vec2 pos_xy, glm::uvec2 const& drawable_size) {

	glm::vec2 pos = pos_xy;

	// based on https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
	hb_buffer_clear_contents(hb_buffer);
	hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
	hb_buffer_guess_segment_properties(hb_buffer);

	hb_shape(hb_font, hb_buffer, NULL, 0);

	uint32_t len = hb_buffer_get_length(hb_buffer);
	hb_glyph_info_t* glyph_infos = hb_buffer_get_glyph_infos(hb_buffer, NULL);
	hb_glyph_position_t* glyph_positions = hb_buffer_get_glyph_positions(hb_buffer, NULL);

	// based on https://learnopengl.com/In-Practice/Text-Rendering (including the for loop below)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(color_texture_program->program);
	glUniform3f(color_texture_program->textColor, 0.5f, 0.8f, 0.2f);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	
	// based on https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text_rendering.cpp
	glm::mat4 projection = glm::ortho(0.0f, (float)drawable_size.x, 0.0f, (float)drawable_size.y);
	glUniformMatrix4fv(color_texture_program->projection, 1, GL_FALSE, &projection[0][0]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (uint32_t i = 0; i < len; i++) {
		// based on https://freetype.org/freetype2/docs/tutorial/step1.html
		hb_codepoint_t glyph_index = glyph_infos[i].codepoint;
		FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT);
		FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                ft_face->glyph->bitmap.width,
                ft_face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                ft_face->glyph->bitmap.buffer
            );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        float w = static_cast<float>(ft_face->glyph->bitmap.width);
        float h = static_cast<float>(ft_face->glyph->bitmap.rows);

		float xpos = pos.x + ft_face->glyph->bitmap_left;
        float ypos = pos.y + ft_face->glyph->bitmap_top - h;

        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
		// render glyph texture over quad
        // glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        pos += glm::vec2(glyph_positions[i].x_advance >> 6, glyph_positions[i].y_advance >> 6); // bitshift by 6 to get value in pixels in both x and y (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}