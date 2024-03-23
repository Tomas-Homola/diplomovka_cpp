#src

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "C:/Qt/${Qt6_Version}/msvc2019_64/lib/cmake/Qt6")
set(Qt6WidgetsTools_DIR "C:/Qt/${Qt6_Version}/msvc2019_64/lib/cmake/Qt6WidgetsTools")
set(Qt6CoreTools_DIR "C:/Qt/${Qt6_Version}/msvc2019_64/lib/cmake/Qt6CoreTools")
set(Qt6GuiTools_DIR "C:/Qt/${Qt6_Version}/msvc2019_64/lib/cmake/Qt6GuiTools")

find_package(Qt6 COMPONENTS Widgets Core Gui Network Concurrent Core5Compat REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/gdal_include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/pdal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/GEOGRAPHICLIB/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/QUAZIP/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/LAStools_include)


file(GLOB H_FILES *.h *.hpp)
file(GLOB CXX_FILES *.cpp)
file(GLOB QRC_FILES *.qrc)

file(GLOB UI_FILES UI/*.ui)

file(GLOB IO_H_FILES IO/*.h)
file(GLOB IO_CXX_FILES IO/*.cpp)

file(GLOB IO_OrthoMosaic_H_FILES IO/OrthoMosaic/*.h)
file(GLOB IO_OrthoMosaic_CXX_FILES IO/OrthoMosaic/*.cpp)

file(GLOB IO_ImageProcessing_H_FILES IP/*.h)
file(GLOB IO_ImageProcessing_CXX_FILES IP/*.cpp)

set(SOURCE_LIST ${CXX_FILES} ${UI_FILES} ${H_FILES} ${QRC_FILES} ${IO_H_FILES} ${IO_CXX_FILES} ${IO_OrthoMosaic_H_FILES} ${IO_OrthoMosaic_CXX_FILES} ${IO_ImageProcessing_H_FILES} ${IO_ImageProcessing_CXX_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Network Qt6::Concurrent Qt6::Core5Compat
${CMAKE_CURRENT_SOURCE_DIR}/../libs/gdal.lib
${CMAKE_CURRENT_SOURCE_DIR}/../libs/pdalcpp.lib
${CMAKE_CURRENT_SOURCE_DIR}/../libs/GEOGRAPHICLIB/GeographicLib-i.lib
${CMAKE_CURRENT_SOURCE_DIR}/../libs/QUAZIP/quazip1-qt6.lib
${CMAKE_CURRENT_SOURCE_DIR}/../libs/LASlib.lib
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

add_custom_command(TARGET ${PROJECT_NAME}
 				POST_BUILD COMMAND "C:/Qt/${Qt6_Version}/msvc2019_64/bin/windeployqt.exe" "$(OutDir)$(TargetName)$(TargetExt)" 
 				COMMENT "Win deploy qt")


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND xcopy /y /d \"$(SolutionDir)..\\libs\\*.dll\" \"$(OutDir)\" COMMENT "gdal_pdal")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND xcopy /y /d \"$(SolutionDir)..\\libs\\proj.db\" \"$(OutDir)\" COMMENT "proj")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND xcopy /y /d \"$(SolutionDir)..\\libs\\proj.db\" \"$(SolutionDir)\\src\" COMMENT "proj")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND xcopy /y /d \"$(SolutionDir)..\\libs\\GEOGRAPHICLIB\\GeographicLib.dll\" \"$(OutDir)\" COMMENT "Geographic")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND xcopy /y /d \"$(SolutionDir)..\\libs\\QUAZIP\\quazip1-qt6.dll\" \"$(OutDir)\" COMMENT "QUAZIP")
