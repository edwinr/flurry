find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(flurry_glfw main.cpp)
target_link_libraries(flurry_glfw PRIVATE flurry_core flurry_opengl glfw OpenGL::GL OpenGL::GLU)
