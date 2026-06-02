# CMakeLists.txt file for library
# To be used within top-level CMakeLists.txt file

# Qt6 requires CMake 3.16 or later
cmake_minimum_required( VERSION 3.16 )

# enable @rpath on MacOS
cmake_policy(SET CMP0042 NEW)
# set linker search policy new
cmake_policy(SET CMP0003 NEW)
# Policy CMP0020: Automatically link Qt executables to qtmain target on Windows.
cmake_policy(SET CMP0020 NEW)


project( SerialPort )

# Qt6 requires C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 modules
find_package(Qt6 COMPONENTS Core SerialPort Concurrent REQUIRED)


if (UNIX)
	ADD_DEFINITIONS( -Wall -fPIC)
ENDIF (UNIX)

include_directories(
	../src
	${Qt6Core_INCLUDE_DIRS}
	${Qt6SerialPort_INCLUDE_DIRS}
)

# run rcc, uic and moc compilers automatically as needed
set(CMAKE_AUTOMOC ON)

# collect a list of all source files of the solver
file( GLOB LIB_SRCS src/*.cpp )

# collect a list of moc header-files in 
file( GLOB LIB_MOC_HDRS src/*.h )

# add build target for library (pass -DBUILD_SHARED_LIBS=ON for a DLL build)
if(BUILD_SHARED_LIBS)
	add_library( ${PROJECT_NAME} SHARED
		${LIB_SRCS}
		${LIB_MOC_HDRS}
	)
	target_compile_definitions( ${PROJECT_NAME} PRIVATE SP_LIBRARY )
else()
	add_library( ${PROJECT_NAME} STATIC
		${LIB_SRCS}
		${LIB_MOC_HDRS}
	)
	target_compile_definitions( ${PROJECT_NAME} PUBLIC SP_STATIC_LIBRARY )
endif()

