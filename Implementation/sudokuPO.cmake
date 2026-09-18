set(sudoku_PO_NAME sudoku)

# ---- Source / header globs ----
file(GLOB sudoku_PO_SOURCES   ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB sudoku_PO_INCS      ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

file(GLOB sudoku_PO_INC_TD      ${MY_INC}/td/*.h)
file(GLOB sudoku_PO_INC_GUI     ${MY_INC}/gui/*.h)
file(GLOB sudoku_PO_INC_THREAD  ${MY_INC}/thread/*.h)
file(GLOB sudoku_PO_INC_CNT     ${MY_INC}/cnt/*.h)
file(GLOB sudoku_PO_INC_FO      ${MY_INC}/fo/*.h)
file(GLOB sudoku_PO_INC_XML     ${MY_INC}/xml/*.h)

# ---- Platform-specific icon / plist ----
set(sudoku_PO_PLIST ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/AppIcon.plist)

if(WIN32)
    set(sudoku_PO_WINAPP_ICON ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/winAppIcon.rc)
else()
    set(sudoku_PO_WINAPP_ICON ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/winAppIcon.cpp)
endif()

# ---- Executable ----
add_executable(${sudoku_PO_NAME}
    ${sudoku_PO_INCS}
    ${sudoku_PO_SOURCES}
    ${sudoku_PO_INC_TD}
    ${sudoku_PO_INC_THREAD}
    ${sudoku_PO_INC_CNT}
    ${sudoku_PO_INC_FO}
    ${sudoku_PO_INC_GUI}
    ${sudoku_PO_INC_XML}
    ${sudoku_PO_WINAPP_ICON}
)

# ---- IDE source groups ----
source_group("inc"         FILES ${sudoku_PO_INCS})
source_group("inc\\td"     FILES ${sudoku_PO_INC_TD})
source_group("inc\\cnt"    FILES ${sudoku_PO_INC_CNT})
source_group("inc\\fo"     FILES ${sudoku_PO_INC_FO})
source_group("inc\\gui"    FILES ${sudoku_PO_INC_GUI})
source_group("inc\\thread" FILES ${sudoku_PO_INC_THREAD})
source_group("inc\\xml"    FILES ${sudoku_PO_INC_XML})
source_group("src"         FILES ${sudoku_PO_SOURCES})

# ---- Link the SDK ----
target_link_libraries(${sudoku_PO_NAME}
    debug     ${MU_LIB_DEBUG}
    debug     ${NATGUI_LIB_DEBUG}
    optimized ${MU_LIB_RELEASE}
    optimized ${NATGUI_LIB_RELEASE}
)

# ---- SDK-provided packaging hooks ----
setTargetPropertiesForGUIApp(${sudoku_PO_NAME} ${sudoku_PO_PLIST})
setAppIcon(${sudoku_PO_NAME} ${CMAKE_CURRENT_LIST_DIR})
setIDEPropertiesForGUIExecutable(${sudoku_PO_NAME} ${CMAKE_CURRENT_LIST_DIR})
setPlatformDLLPath(${sudoku_PO_NAME})
