-- imui

local imui_path = module.config.base_path

tiki.use_sdl = (tiki.target_platform == Platforms.Windows)
tiki.use_lib = false

module.module_type = ModuleTypes.UnityCModule

module:add_include_dir( "include" )

module:add_files( "include/imui/*.h" )
module:add_files( "src/*.h" )
module:add_files( "src/*.c" )

--module:add_external( "https://github.com/Immediate-Mode-UI/Nuklear.git" )
--module:add_external( "https://github.com/KhronosGroup/Vulkan-Headers.git@sdk-1.2.189" )
--module:add_external( "https://github.com/lvandeve/lodepng.git" )
