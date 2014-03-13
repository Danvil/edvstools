PROJECT(EdvsMathLink)

set(MLDIR "/usr/local/Wolfram/Mathematica/9.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64" CACHE PATH "Mathematica MathLink Kit Base Directory")

ADD_DEFINITIONS(-m64 -std=c99)

INCLUDE_DIRECTORIES(
	${edvstools_SOURCE_DIR}
	${MLDIR}/CompilerAdditions
)

LINK_DIRECTORIES(
	${MLDIR}/CompilerAdditions
)

ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_BINARY_DIR}/MathLink/EdvsMathLink_tm.c
	COMMAND ${MLDIR}/CompilerAdditions/mprep ${edvstools_SOURCE_DIR}/MathLink/EdvsMathLink.tm -o ${CMAKE_BINARY_DIR}/MathLink/EdvsMathLink_tm.c
	DEPENDS ${edvstools_SOURCE_DIR}/MathLink/EdvsMathLink.tm
)

ADD_EXECUTABLE(${PROJECT_NAME}
	${CMAKE_BINARY_DIR}/MathLink/EdvsMathLink_tm.c
	main.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	Edvs
	ML64i3.a
	m
	pthread
	rt
	stdc++
)

