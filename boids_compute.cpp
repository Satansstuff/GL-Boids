#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <random>
#define num_boids 48
std::string loadtext(std::string in)
{
    std::ifstream t(in);
    std::string str;

    t.seekg(0, std::ios::end);   
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());
    return str;
}
void checkShader(unsigned shader)
{
    int  success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cout << "Failed to compile: " << log << std::endl;
        std::exit(-1);
    }
}
void link(unsigned program)
{
    glLinkProgram(program);
    int success = false;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char log[512];
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cout << log << std::endl;
        std::exit(-1);
    }
}
int main(int argc, char **argv)
{

    glfwInit();
    bool is_open = true;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Boids - GPU",  nullptr, nullptr); // Windowed
    if(!window)
    {
        glfwTerminate();
        std::cerr << "Failed to open window " << std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init glew" << std::endl;
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, 800, 600);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    // Randomized data
    
    glm::vec2 positon[num_boids];
    glm::vec2 velocity[num_boids];
    {
        std::default_random_engine generator;
        std::uniform_real_distribution<float> x(-50.0,50.0);
        std::uniform_real_distribution<float> y(-50.0,50.0);
        std::uniform_real_distribution<float> v(0.001,0.3);
        for(unsigned i = 0; i < num_boids; i++)
        {
            positon[i].x = x(generator);
            positon[i].y = y(generator);
            
            velocity[i].x = v(generator);
            velocity[i].y = v(generator);
        }
    }

    // OpenGL SHIT

    GLuint vao, positions, velocities;
    glGenBuffers(1, &positions);
    glGenBuffers(1, &velocities);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Velocities
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocities);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * num_boids, velocity, GL_DYNAMIC_DRAW);
    
    //Position buffer / VBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positions);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * num_boids, positon, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, positions);

    GLint posAttrib;
    unsigned renderprogram,computeprogram;
    unsigned vs, fs, gs, cs;
    {

        auto vert_text = loadtext("vert.glsl");
        auto frag_text = loadtext("frag.glsl");
        auto gs_text = loadtext("geom.glsl");
        auto comp_text = loadtext("compute.comp");
        auto rule1_text = loadtext("rule1.comp");
        auto rule2_text = loadtext("rule2.comp");
        auto rule3_text = loadtext("rule3.comp");
        vs = glCreateShader(GL_VERTEX_SHADER);
        const char *src = vert_text.c_str();
        glShaderSource(vs, 1, &src, NULL);
        std::cout << "OpenGL >> Compiling Vertex shader.." << std::endl;
        glCompileShader(vs);
        checkShader(vs);
        fs = glCreateShader(GL_FRAGMENT_SHADER);
        src = frag_text.c_str();
        glShaderSource(fs, 1, &src, NULL);
        std::cout << "OpenGL >> Compiling Fragment shader.." << std::endl;
        glCompileShader(fs);
        checkShader(fs);
        gs = glCreateShader(GL_GEOMETRY_SHADER);
        src = gs_text.c_str();
        glShaderSource(gs, 1, &src, NULL);
        std::cout << "OpenGL >> Compiling Geometry shader" << std::endl;
        glCompileShader(gs);
        checkShader(gs);
        cs = glCreateShader(GL_COMPUTE_SHADER);
        src = comp_text.c_str();
        glShaderSource(cs, 1, &src, NULL);
        std::cout << "OpenGL >> Compiling compute shader.." << std::endl;
        glCompileShader(cs);
        checkShader(cs);
  
        renderprogram = glCreateProgram();
        computeprogram = glCreateProgram();

        glAttachShader(renderprogram, vs);
        glAttachShader(renderprogram, fs);

        glAttachShader(computeprogram, cs);

        glAttachShader(renderprogram, vs);
        glAttachShader(renderprogram, fs);
        //glAttachShader(renderprogram, gs);
        std::cout << "OpenGL >> Linking Renderprogram" << std::endl;
        link(renderprogram);
        posAttrib = glGetAttribLocation(renderprogram, "position");
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posAttrib);
        std::cout << "OpenGL >> Linking Compute-shader" << std::endl;
        link(computeprogram);
    }
    glUseProgram(renderprogram);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
    
    glm::mat4 vm = glm::lookAt(glm::vec3(0.0, 0.0, -5), 
        glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    
    glm::mat4 pm = glm::perspective(60.0f, 4.0f / 3.0f, 0.0001f, 1000.0f);

    glUseProgram(renderprogram);

    unsigned vlp, plp;
    vlp = glGetUniformLocation(renderprogram, "view");
    plp = glGetUniformLocation(renderprogram, "projection");
    double current = glfwGetTime(), prev = current;
    double delta = 0.0;
    while(is_open && !glfwWindowShouldClose(window))
    {
        current = glfwGetTime();
        delta = current - prev;
        if(delta >= 1.0/32.0)
        {
            
            glUseProgram(computeprogram);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positions);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocities);
            glDispatchCompute(num_boids, 1, 1);
            glUseProgram(0);
            prev = current;
        }
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glUseProgram(renderprogram);
        glUniformMatrix4fv(vlp, 1, GL_FALSE, &vm[0][0]);
        glUniformMatrix4fv(plp, 1, GL_FALSE, &pm[0][0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, num_boids);
        glfwSwapBuffers(window);
        glfwPollEvents();
        glUseProgram(0);
    }
    glfwTerminate();
    return 0;
}