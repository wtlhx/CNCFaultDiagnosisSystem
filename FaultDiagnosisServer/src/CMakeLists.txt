file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/FaultDiagnosisServer/src/*.cpp)

add_library(FaultDiagnosisServer_shared SHARED ${SOURCES})