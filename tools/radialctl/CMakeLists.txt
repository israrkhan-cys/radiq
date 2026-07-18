qt_add_executable(radialctl
    main.cpp
)

target_link_libraries(radialctl PRIVATE
    Qt6::Core
    Qt6::Network
)
