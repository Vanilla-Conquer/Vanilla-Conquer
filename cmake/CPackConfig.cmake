# Based on https://github.com/percolator/percolator/blob/763d90c137f4ee6f0d4f19be4aeb7e59472b5ef5/CPack.txt

# PACKAGING OPTIONS: GENERAL
set(VANILLA_INSTALL_GUID "a054cfaa-a5bf-482a-bb2c-648ec58bf5e4")
set(CPACK_SET_DESTDIR TRUE) # Enabling absolute paths for CPack (important!)
set(CPACK_SOURCE_GENERATOR "TGZ") # This file format is used to package source code ("make package_source")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/")
set(CPACK_PACKAGE_VENDOR "Assembly Armada")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

if(BUILD_VANILLATD)
    list(APPEND CPACK_COMPONENTS_ALL VanillaTD)
endif()

if(BUILD_VANILLARA)
    list(APPEND CPACK_COMPONENTS_ALL VanillaRA)
endif()

if(WIN32)
    set(CPACK_PACKAGE_NAME "Vanilla-Conquer")
    list(APPEND CPACK_COMPONENTS_ALL RuntimeDeps)
    if(BUILD_VANILLATD)
        set(CPACK_COMPONENT_VanillaTD_DEPENDS RuntimeDeps)
    endif()
    if(BUILD_VANILLARA)
        set(CPACK_COMPONENT_VanillaRA_DEPENDS RuntimeDeps)
    endif()
    set(CPACK_COMPONENT_RuntimeDeps_HIDDEN TRUE)
    set(CPACK_COMPONENT_RuntimeDeps_REQUIRED TRUE)
else()
    set(CPACK_PACKAGE_NAME "vanilla-conquer")
endif()

if(WIN32)
    set(CPACK_SET_DESTDIR OFF)
    set(CPACK_GENERATOR "WIX")
    set(CPACK_WIX_UPGRADE_GUID ${VANILLA_INSTALL_GUID})
    
    set_property(INSTALL "bin/$<TARGET_FILE_NAME:VanillaTD>"
        PROPERTY CPACK_START_MENU_SHORTCUTS "Vanilla TD"
    )
    
    set_property(INSTALL "bin/$<TARGET_FILE_NAME:VanillaRA>"
        PROPERTY CPACK_START_MENU_SHORTCUTS "Vanilla RA"
    )
endif()

if(UNIX)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SPECIFIC_SYSTEM_VERSION_NAME "${CMAKE_SYSTEM_NAME}")
    set(CPACK_GENERATOR "TGZ")
    if(EXISTS "/etc/redhat-release")
      set(LINUX_NAME "")
      file(READ "/etc/redhat-release" LINUX_ISSUE)
    elseif(EXISTS "/etc/issue")
      set(LINUX_NAME "")
      file(READ "/etc/issue" LINUX_ISSUE)
    endif()
    if(DEFINED LINUX_ISSUE)
      # Fedora case
      if(LINUX_ISSUE MATCHES "Fedora")
        string(REGEX MATCH "release ([0-9]+)" FEDORA "${LINUX_ISSUE}")
        set(LINUX_NAME "FC${CMAKE_MATCH_1}")
        set(CPACK_GENERATOR "RPM")
      endif(LINUX_ISSUE MATCHES "Fedora")
      # Red Hat case
      if(LINUX_ISSUE MATCHES "Red")
        string(REGEX MATCH "release ([0-9]+\\.[0-9]+)" REDHAT "${LINUX_ISSUE}")
        set(LINUX_NAME "RHEL_${CMAKE_MATCH_1}")
        set(CPACK_GENERATOR "RPM")
      endif(LINUX_ISSUE MATCHES "Red")
      # CentOS case
      if(LINUX_ISSUE MATCHES "CentOS")
        string(REGEX MATCH "release ([0-9]+\\.[0-9]+)" CENTOS "${LINUX_ISSUE}")
        set(LINUX_NAME "CentOS_${CMAKE_MATCH_1}")
        set(CPACK_GENERATOR "RPM")
      endif(LINUX_ISSUE MATCHES "CentOS")
      # Ubuntu case
      if(LINUX_ISSUE MATCHES "Ubuntu")
        string(REGEX MATCH "buntu ([0-9]+\\.[0-9]+)" UBUNTU "${LINUX_ISSUE}")
        set(LINUX_NAME "Ubuntu_${CMAKE_MATCH_1}")
        set(CPACK_GENERATOR "DEB")
      endif(LINUX_ISSUE MATCHES "Ubuntu")
      # Debian case
      if(LINUX_ISSUE MATCHES "Debian")
        string(REGEX MATCH "Debian .*ux ([a-zA-Z]*/?[a-zA-Z]*) .*" DEBIAN "${LINUX_ISSUE}")
        set(LINUX_NAME "Debian_${CMAKE_MATCH_1}")
        string(REPLACE "/" "_" LINUX_NAME ${LINUX_NAME})
        set(CPACK_GENERATOR "DEB")
      endif(LINUX_ISSUE MATCHES "Debian")
      # Open SuSE case
      if(LINUX_ISSUE MATCHES "SUSE")
        string(REGEX MATCH "SUSE  ([0-9]+\\.[0-9]+)" SUSE "${LINUX_ISSUE}")
        set(LINUX_NAME "openSUSE_${CMAKE_MATCH_1}")
        string(REPLACE "/" "_" LINUX_NAME ${LINUX_NAME})
        set(CPACK_GENERATOR "RPM")
      endif(LINUX_ISSUE MATCHES "SUSE")
    endif(DEFINED LINUX_ISSUE)
  endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
endif(UNIX)

set(CPACK_STRIP_FILES TRUE)

file(GLOB_RECURSE DOT_FILES_BEGIN ".*") # To be ignored by Cpack
file(GLOB_RECURSE TILD_FILES "*~*") # To be ignored by Cpack

set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;/.swp$/;cscope.*;/.git/;${CMAKE_CURRENT_BINARY_DIR}/;/.bzr/;/.settings/;${DOT_FILES_BEGIN};${TILD_FILES}")

# PACKAGING OPTIONS: DEB
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Assembly Armada")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${TARGET_ARCH})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0 (>= 2.0.5), libopenal1 (>= 1.17)")

# PACKAGING OPTIONS: RPM
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3 license")
set(CPACK_RPM_PACKAGE_VENDOR "Assembly Armada")

include(CPack)
