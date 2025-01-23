add_executable(task_plan_test task_plan_test.cpp)

target_link_libraries(task_plan_test PUBLIC roco2_core)

add_test(task_plan_test task_plan_test)
