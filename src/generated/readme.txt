Here you find the source files genereated by flex on Unix to be able to build
YAM on AmigaOS4. Unfortunately the natively generated files cause crashes due
to the too old flex version on AmigaOS.

These files must be copied to the parent directory (where the corresponding *.l
files can be found). The build process will then skip the generation as long as
the generated *.c files have a more recent date than the *.l files.
