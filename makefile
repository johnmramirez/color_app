  all: Source/color_app.c 
		gcc -o color_app.exe -I/home/jaj/Desktop/color_app/Includes Source/color_app.c Source/display.c -l:libX11.dll.a
  clean: 
		$(RM) color_app.exe