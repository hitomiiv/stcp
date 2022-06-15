cmake_minimum_required(VERSION 3.12)

add_executable(driver driver.c)
target_link_libraries(driver PRIVATE stcp)

add_test(NAME Driver COMMAND driver)