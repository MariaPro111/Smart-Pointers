# ------------------------------------------------------------------------------
# UniquePtr

add_catch(test_unique unique/test.cpp)

# ------------------------------------------------------------------------------
# SharedPtr + WeakPtr

add_catch(test_shared
    shared/test.cpp)

add_catch(test_weak
    weak/test.cpp
    weak/test_shared.cpp
    weak/test_odr.cpp)


target_link_libraries(test_shared allocations_checker)
target_link_libraries(test_weak allocations_checker)

# ------------------------------------------------------------------------------
# IntrusivePtr

add_catch(test_intrusive intrusive/test.cpp)
target_link_libraries(test_intrusive allocations_checker)
