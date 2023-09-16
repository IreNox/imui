local module = Module:new()

module:add_files( "*.h" )
module:add_files( "*.c" )

module:add_external( "https://github.com/libsdl-org/SDL@2.28.3" )

if tiki.target_platform == Platforms.Windows then
	module:add_external( "https://github.com/nigels-com/glew@2.2.0" )

	module:add_library_file( "opengl32" )
	
	module.import_func = function( project, solution )
		project:set_flag( "MultiProcessorCompile" )
	end
end
