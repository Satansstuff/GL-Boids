#version 430

layout(points) in;
layout(line_strip, max_vertices = 200) out;

void main()
{
    vec4 pos = gl_in[0].gl_Position;
    for(float i = 0; i < 6.38 ; i+=0.1)
    {
        gl_Position = vec4(pos.x+0.03*cos(i),pos.y+0.03*sin(i),pos.z,1.0);
        EmitVertex();      
    }
}