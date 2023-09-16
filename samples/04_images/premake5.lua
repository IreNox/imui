-- samples/04_images

local project = Project:new( ProjectTypes.WindowApplication )

project:add_files( 'src/*.c' )

project:add_dependency( "framework" )

project:add_external( "local://../.." )

project:add_post_build_step( "copy_files", { pattern = "assets/**" } )

finalize_default_solution( project )
