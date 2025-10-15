
#定义了一个src_list变量，包含了该目录下的所有源文件
aux_source_directory(. SRC_LIST)


add_executable(ChatClient ${SRC_LIST})

target_link_libraries(ChatClient mysqlclient ssl crypto dl pthread)