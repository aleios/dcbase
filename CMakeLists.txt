cmake_minimum_required(VERSION 3.21)
project(dcbase)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Environment: $ENV{KOS_BASE}")

# ---- Guard against improperly configured environment ----
if(NOT DEFINED ENV{KOS_BASE})
    message(FATAL_ERROR "KOS environment is not setup properly. Make sure to source environ.sh first!")
endif()

# ---- Main target ----
add_executable(${PROJECT_NAME} stb.c main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
                      CMAKE_CXX_STANDARD 17
                      CXX_EXTENSIONS OFF
                      )
target_link_libraries(${PROJECT_NAME} PUBLIC GLdc)


# ---- ROMDISK generation ----
set(GENROMFS "$ENV{KOS_BASE}/utils/genromfs/genromfs")
set(BIN2O $ENV{KOS_BASE}/utils/bin2o/bin2o)
set(ROMDISK_IMG "${CMAKE_CURRENT_BINARY_DIR}/romdisk.img")
set(ROMDISK_O "${CMAKE_CURRENT_BINARY_DIR}/romdisk.o")
set(ROMDISK_DIR "${CMAKE_SOURCE_DIR}/romdisk")


# Generate ROMDISK. Make dependency of main target so it gets (re)built before linkage.
# TODO: Better way to handle this? Only generate when romdisk changes?
add_custom_target(romdisk.o ALL
                  COMMAND ${GENROMFS} -f ${ROMDISK_IMG} -d ${ROMDISK_DIR} -v
                  COMMAND ${BIN2O} ${ROMDISK_IMG} romdisk ${ROMDISK_O}
                  COMMENT "Generating romdisk")

add_dependencies(${PROJECT_NAME} romdisk.o)
target_link_libraries(${PROJECT_NAME} PUBLIC ${ROMDISK_O})

# ---- CDI Generation ----
# After main target linkage. Only executed when .cdi target specified.
add_custom_target(${PROJECT_NAME}.cdi ALL
                  COMMAND $ENV{KOS_OBJCOPY} -R .stack -O binary -S -g ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
                  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-outputs
                  COMMAND $ENV{KOS_BASE}/utils/scramble/scramble ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-outputs/1ST_READ.BIN

                  # Generate IP.BIN
                  COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/IP.BIN
                  COMMAND $ENV{KOS_BASE}/utils/makeip/makeip ${CMAKE_CURRENT_SOURCE_DIR}/IP.txt ${CMAKE_CURRENT_BINARY_DIR}/IP.BIN

                  # Build iso using ${PROJECT_NAME} subdir as volume root.
                  COMMAND mkisofs -C 0,11702 -V ${PROJECT_NAME} -G ${CMAKE_CURRENT_BINARY_DIR}/IP.BIN -r -J -l -o ${PROJECT_NAME}.iso ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-outputs

                  # Generate cdi image
                  COMMAND $ENV{KOS_BASE}/utils/cdi4dc/cdi4dc ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.iso ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cdi
                  DEPENDS ${PROJECT_NAME}
                  COMMENT "Making CDI image"
                  )