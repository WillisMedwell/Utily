cmake_minimum_required(VERSION 3.11)
project(UtilyExample)

# include(FetchContent)
# FetchContent_Declare(
#     Utily
#     GIT_REPOSITORY https://github.com/WillisMedwell/Utily.git
#     GIT_TAG main
#     GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(Utily)

add_executable(UtilyExample ${CMAKE_CURRENT_SOURCE_DIR}/src/example_main.cpp)
target_link_libraries(UtilyExample PRIVATE Utily::Utily)
