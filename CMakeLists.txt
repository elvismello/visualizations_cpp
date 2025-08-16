cmake_minimum_required(VERSION 3.25)
project(visProject LANGUAGES CXX)

# Configurar padrão C++
set(DCMAKE_CXX_COMPILER g++-13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Diretórios de include
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/include) # ajuste se necessário

# Encontrar bibliotecas
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)

# Adicionar arquivos fonte
file(GLOB SOURCES src/*.cpp)

# Criar executável
add_executable(${PROJECT_NAME} ${SOURCES})

# Linkar bibliotecas
target_link_libraries(${PROJECT_NAME}
    OpenCL::OpenCL
    OpenGL::GL
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    dl
    pthread
    X11
)

# Incluir diretórios das bibliotecas
target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# Copiar kernels para a pasta de build
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kernels)
file(GLOB KERNELS kernels/*.cl)
foreach(k ${KERNELS})
    configure_file(${k} ${CMAKE_CURRENT_BINARY_DIR}/kernels/ COPYONLY)
endforeach()

# Copiar shaders para a pasta de build
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(GLOB SHADERS shaders/*.glsl)
foreach(k ${SHADERS})
    configure_file(${k} ${CMAKE_CURRENT_BINARY_DIR}/shaders/ COPYONLY)
endforeach()