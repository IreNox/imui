-- samples/01_hello_world

add_module_include_path( ".." )

local project = Project:new( ProjectTypes.WindowApplication )

project:add_files( 'src/*.c' )

project:add_dependency( "framework" )

project:add_external( "local://../.." )

finalize_default_solution( project )
