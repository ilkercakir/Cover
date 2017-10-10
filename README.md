# Cover
Cover video and sound files on the fly with music instruments connected to the microphone input of your PC.

Uses ALSA inputs and processes samples in almost real time using small periods of 128 frames. Supports mono / stereo microphone inputs. Audio samples from the mic are processed immediately using digital haas and modulation effects to generate stereo image, and played back with minimum delay. Audio samples coming from the built in audio / video player based on ffmpeg library are first processed by the biquad audio equalizer, and then digitally mixed with audio input from mic. Microphone input can be processed with delay and reverb effects. The reverb effect relies on combining multiple delay lines.
The resulting output is played on speaker output. Allows selection of input and output hardware devices.
The media player has playlist feature, based on sqlite3 library to save the playlist in a local sqlite database. Drag and drop on the playlist to add files is supported.
Video player uses OpenGL ES2 to use the GPU for rendering video frames on a GTK+3 drawing area. GtkGLArea is not yet available for the Raspberry Pi, instead a single buffered GTK+3 drawing area was used as canvas.
Tested on Ubuntu running on an old Compaq nc4120 with single core Pentium M CPU, a recent HP 430 with 4 cores i5 CPU, and Raspbian Jessie distribution on Raspberry Pi 3 ARMv8 4 cores CPU.
Please don't forget to remove ARM related flags from compiler command line when recompiling for other hardware. Makefile is for Raspberry Pi.
