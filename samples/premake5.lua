-- samples

local project = Project:new( ProjectTypes.WindowApplication )

project:add_files( 'src/*.h' )
project:add_files( 'src/*.c' )
project:add_files( 'src/*.cpp' )

project:add_external( "local://.." )

project:add_external( "https://github.com/libsdl-org/SDL@2.28.3" )

if tiki.target_platform == Platforms.Windows then
	project:add_external( "https://github.com/nigels-com/glew@2.2.0" )

	project:add_library_file( "opengl32" )

	project:add_define( "_CRT_SECURE_NO_WARNINGS", "1" );
	
	project.import_func = function( project, solution )
		project:set_flag( "MultiProcessorCompile" )
	end
end

finalize_default_solution( project )
