sdl-shadow-mapping

Author:

	Jurgen Schober
	
Date:
   
	January, 2013
	
Short:
  
	Shadow mapping tutorial - work in progress

Description:

	Based on sdl-vertex-animation this example uses code from previous sdl-xx-examples I've written.
	
	Tutorials can be found here:
	
	   Traditional non-shader implementation:
	       http://www.paulsprojects.net/tutorials/smt/smt.html
	       
	   More sophisticated tutorials using shaders
	       http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
	
	New additions:
	
	- Interleaved vertex arrays (combine vertex with texture vectors)
	- New brush loader simplifies loading of textures
	- Some cleanup in the init code
	- Textures can control filter and wrap mode
	
	All sdl-xx-examples are written in C++11 using MinGW gcc 4.6 and are Windows only. I'm using
	Eclipse Juno as Development IDE.

Libs used:

	boost_thread
	boost_system
	boost_filesystem
	boost_iostreams
	glew
	+ OpenGL

License:

	Use as is. No license other then the ones included with third party libraries are required.

Compiler used:

	MinGW with Eclipse Juno (gcc4.6 with -std=c++0x). Windows only. Linux might just work, MacOS will 
	need some work due to the fact OSX needs to run the render loop in the main loop (compiles with 
	LVM compiler). This example runs a render thread decoupled from the main thread.

Have fun
Jurgen
