layout(location = 0) in vec2 position_MODELSPACE;


uniform mat4 ortho;
void main() {
	gl_Position = ortho * vec4(position_MODELSPACE,-0.01,1.0);
}
