# Cover
Cover video and sound files on the fly with music instruments connected to the microphone input of your PC.

Cover uses ALSA inputs and processes samples in almost real time using small periods of 128 frames. Supports mono / stereo microphone inputs. Audio samples from the mic are processed immediately using digital haas and modulation effects to generate stereo image, and played back with minimum delay. Audio samples coming from the built in audio / video player based on ffmpeg library are first processed by the biquad audio equalizer, and then digitally mixed with audio input from mic. Microphone input can be processed with delay and reverb effects. The reverb effect relies on combining multiple delay lines.
The resulting output is played on speaker output. Allows selection of input and output hardware devices.
The media player has playlist feature, based on sqlite3 library to save the playlist in a local sqlite database. Drag and drop on the playlist to add files is supported.
Video player uses OpenGL ES2 to use the GPU for rendering video frames on a GTK+3 drawing area. GtkGLArea is not yet available for the Raspberry Pi, instead a single buffered GTK+3 drawing area was used as canvas.
Tested on Ubuntu running on an old Compaq nc4120 with single core Pentium M CPU, a recent HP 430 with 4 cores i5 CPU, and Raspbian Jessie distribution on Raspberry Pi 3 ARMv8 4 cores CPU.
Please don't forget to remove ARM related flags from compiler command line when recompiling for other hardware. Makefile is for Raspberry Pi.
Specific to the Pi : You will need to use an external USB mic with the Pi. Pi's USB 2 drivers in isochronous mode are somewhat problematic if CPU cannot respond quickly, i.e. under load, which is the case when playing a video. A workaround is to use the on board audio output with a USB mic input (but not the USB audio in/out at the same time).

How I typically use this software : I connect my electric guitar to ZOOM 1010 effects processor, feed the output of the pedal to the mic input of my PC, play my favourite band's live mp4 recording on Cover, and let Cover do the audio processing to convert mono input from the guitar to stereo and mix with the audio from mp4 file in almost real time.
