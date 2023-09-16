-- imui

module.module_type = ModuleTypes.FilesModule

module:add_include_dir( "include" )

module:add_files( "include/imui/*.h" )
module:add_files( "src/*.h" )
module:add_files( "src/*.c" )
