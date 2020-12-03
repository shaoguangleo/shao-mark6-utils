cmake_minimum_required(VERSION 3.2)

install(CODE "MESSAGE (\"Ready to install scripts\")")
install(PROGRAMS shao_mark6_mount.sh DESTINATION bin)
execute_process(COMMAND ln -s /usr/local/bin/shao_mark6_mount.sh /usr/local/bin/shao_mark6_mount)

install(PROGRAMS shao_mark6_gather_files.py DESTINATION bin)
execute_process(COMMAND ln -s /usr/local/bin/shao_mark6_gather_files.sh /usr/local/bin/shao_mark6_gather_files)

install(PROGRAMS shao_mark6_dethread_files.py DESTINATION bin)
execute_process(COMMAND ln -s /usr/local/bin/shao_mark6_dethread_files.sh /usr/local/bin/shao_mark6_dethread_files)

install(PROGRAMS shao_mark6_list_files.py DESTINATION bin)
execute_process(COMMAND ln -s /usr/local/bin/shao_mark6_list_files.sh /usr/local/bin/shao_mark6_list_files)