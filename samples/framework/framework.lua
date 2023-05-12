local module = Module:new()

module:add_files( "*.h" )
module:add_files( "*.c" )

module:add_external( "https://www.libsdl.org@2.0.12" )

if tiki.target_platform == Platforms.Windows then
	module:add_external( "https://glew.sourceforge.net/@2.1.0" )

	module:add_library_file( "opengl32" )
	
	module.import_func = function( project, solution )
		project:set_flag( "MultiProcessorCompile" )
	end
end
