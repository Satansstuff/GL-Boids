#version 430
#define NUM_BOIDS 48
#define FLOCK_SIZE 8
in vec2 position;
out vec3 color;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    int flock = gl_VertexID % FLOCK_SIZE;
    color = flock * vec3(0.1,0.1,0.5);
    gl_PointSize = 5.0;
    gl_Position = projection * view * vec4(position, 0.0f , 1.0f);
}