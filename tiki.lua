-- imui

tiki.use_sdl = (tiki.target_platform == Platforms.Windows)
tiki.use_lib = false

module.module_type = ModuleTypes.UnityCModule

module:add_include_dir( "include" )

module:add_files( "include/imui/*.h" )
module:add_files( "src/*.h" )
module:add_files( "src/*.c" )
