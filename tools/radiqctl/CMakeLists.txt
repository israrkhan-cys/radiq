qt_add_executable(radiqctl
    main.cpp
)

target_link_libraries(radiqctl PRIVATE
    Qt6::Core
    Qt6::Network
)
