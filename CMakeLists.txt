set(SRCS
    Channel.cpp
    Epoll.cpp
    EventLoop.cpp
    Thread.cpp
    EventLoopThreadPool.cpp
    HttpServer.cpp
    HttpConnection.cpp
    ListenSocket.cpp
    main.cpp
    NotifyEvent.cpp
    Server.cpp
    Timer.cpp
    TimerManager.cpp
    Util.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/base)

add_executable(WebServer ${SRCS})

target_link_libraries(WebServer libserver_base)

add_subdirectory(base)