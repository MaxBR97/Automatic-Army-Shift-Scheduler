cmd_Makefile := cd ..; /usr/local/lib/node_modules/node-gyp/gyp/gyp_main.py -fmake --ignore-environment "-Dlibrary=shared_library" "-Dvisibility=default" "-Dnode_root_dir=/home/spl211/.cache/node-gyp/16.14.0" "-Dnode_gyp_dir=/usr/local/lib/node_modules/node-gyp" "-Dnode_lib_file=/home/spl211/.cache/node-gyp/16.14.0/<(target_arch)/node.lib" "-Dmodule_root_dir=/home/spl211/Downloads/Army_Shcedule_App/Automatic-Army-Shift-Scheduler" "-Dnode_engine=v8" "--depth=." "-Goutput_dir=." "--generator-output=build" -I/home/spl211/Downloads/Army_Shcedule_App/Automatic-Army-Shift-Scheduler/build/config.gypi -I/usr/local/lib/node_modules/node-gyp/addon.gypi -I/home/spl211/.cache/node-gyp/16.14.0/include/node/common.gypi "--toplevel-dir=." binding.gyp
