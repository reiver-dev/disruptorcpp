
add_definitions(-std=c++11)

add_executable ( perf-1P-1EP-UNICAST one_publisher_to_one_unicast_throughput_test.cc )
target_link_libraries(perf-1P-1EP-UNICAST pthread)

add_executable ( perf-1P-3EP-PIPELINE one_publisher_to_three_pipeline_throughput_test.cc )
target_link_libraries(perf-1P-3EP-PIPELINE pthread)
