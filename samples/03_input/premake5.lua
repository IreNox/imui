-- samples/hello_world

add_module_include_path( ".." )

local project = Project:new( "imapp_layout", ProjectTypes.WindowApplication )

project:add_files( 'src/*.c' )

project:add_dependency( "framework" )

project:add_external( "local://../.." )

project:add_define( "_CRT_SECURE_NO_WARNINGS", "1" );

finalize_default_solution( project )
