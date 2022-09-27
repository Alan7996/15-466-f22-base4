#include "ColorTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Load< ColorTextureProgram > color_texture_program(LoadTagEarly);

ColorTextureProgram::ColorTextureProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	//based on https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text.fs as pointed out to me by David Lyons
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"uniform mat4 projection;\n"
		"in vec4 Position;\n"
		"out vec2 texCoord;\n"
		"void main() {\n"
		"	gl_Position = projection * vec4(Position.xy, 0.0, 1.0);\n"
		"	texCoord = Position.zw;\n"
		"}\n"
	,
		//fragment shader:
		"#version 330\n"
		"uniform sampler2D TEX;\n"
		"uniform vec3 textColorUni;\n"
		"in vec2 texCoord;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(TEX, texCoord).r);\n"
    	"	color = vec4(textColorUni, 1.0) * sampled;\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "Position");
	TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

	//look up the locations of uniforms:
	projection = glGetUniformLocation(program, "projection");
	GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");
	textColor = glGetUniformLocation(program, "textColorUni");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

ColorTextureProgram::~ColorTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

