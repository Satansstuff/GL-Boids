  # the compiler: gcc for C program, define as g++ for C++
  CC = g++

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g 

  # the build target executable:
  TARGET = boids_compute

  all: $(TARGET)

  $(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp -lglfw3 -lGL -ldl -pthread -lGLEW -lOpenCL

  clean:
	$(RM) $(TARGET)
