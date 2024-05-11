cmake_minimum_required(VERSION 3.16)

# --------- QTaskList ---------

# Define the project and language
project(QTaskList VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set up widget source files
set(SOURCES 
    src/task_list.cpp
	src/task_filter.cpp
	src/task_utils.cpp
)

# Set widget header files
set(HEADERS
    src/task_list.h
	src/task_filter.h
	src/task_utils.h
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# Set up include directories for consumers of this library
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Translations
# Set translations sources.
set(TS_SOURCE_fr_FR 
	"${CMAKE_CURRENT_SOURCE_DIR}/resources/ts/${PROJECT_NAME}_fr_FR.ts"
)

# Set the directory where .qm files will be generated.
set_source_files_properties(${TS_SOURCE_fr_FR}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations"
)

# --------- Qt6 ---------
find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)


# Link Qt modules to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE
	Qt6::Widgets
	Qt6::Core
	Qt6::Gui
 )
