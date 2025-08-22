cmake_minimum_required(VERSION 3.10)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
if (MSVC)
   # Turn off Microsofts "security" warnings.
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS NDEBUG _CONSOLE _SCL_SECURE_NO_WARNINGS NOGDICAPMASKS NOVIRTUALKEYCODES NOWINMESSAGES NOWINSTYLES NOSYSMETRICS NOMENUS NOICONS NOKEYSTATES NOSYSCOMMANDS NORASTEROPS NOSHOWWINDOW OEMRESOURCE NOATOM NOCLIPBOARD NOCOLOR NOCTLMGR NODRAWTEXT NOGDI NOKERNEL NOUSER NONLS NOMB NOMEMMGR NOMETAFILE NOMINMAX NOMSG NOOPENFILE NOSCROLL NOSOUND NOTEXTMETRIC NOWH NOWINOFFSETS NOCOMM NOKANJI NOHELP NOPROFILER NODEFERWINDOWPOS NOMCX)
  add_compile_options(/EHa /MT$<$<CONFIG:Debug>:d>)
else()
  add_compile_options(-Wall)
endif()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/SocketLib)

add_subdirectory(SocketLib EXCLUDE_FROM_ALL)

if(UA_ENABLE_ENCRYPTION_OPENSSL)
  link_directories(${OPENSSL_LIB_DIR})
endif()

if(UA_ENABLE_ENCRYPTION_OPENSSL)
  set(LIBCRYPTO "crypto")
  set(LIBSSL "ssl")
  if (MSVC)
    set(LIBCRYPTO "libcrypto")
    set(LIBSSL "libssl")
  endif()
else()
  set(LIBCRYPTO "")
  set(LIBSSL "")
endif()

add_executable(Tcp-Example Tcp-Example.cpp)
target_link_libraries(Tcp-Example socketlib ${LIBCRYPTO} ${LIBSSL})

add_executable(Ssl-Example Ssl-Example.cpp)
target_link_libraries(Ssl-Example socketlib ${LIBCRYPTO} ${LIBSSL})

add_executable(StartTls-Example StartTls-Example.cpp)
target_link_libraries(StartTls-Example socketlib ${LIBCRYPTO} ${LIBSSL})

add_executable(Dtls-Example Dtls-Example.cpp)
target_link_libraries(Dtls-Example socketlib ${LIBCRYPTO} ${LIBSSL})

add_executable(Notify-Example Notify-Example.cpp)
target_link_libraries(Notify-Example socketlib ${LIBCRYPTO} ${LIBSSL})

if(NOT MSVC AND UA_ENABLE_ENCRYPTION_OPENSSL)
  target_link_libraries(Tcp-Example pthread)
  target_link_libraries(Ssl-Example pthread)
  target_link_libraries(StartTls-Example pthread)
  target_link_libraries(Dtls-Example pthread)
  target_link_libraries(Notify-Example pthread)
endif()
