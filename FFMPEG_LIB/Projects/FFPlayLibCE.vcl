<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: FFPlayLibCE - Win32 (WCE ARMV4I) Release Fixpoint--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\ms\LOCALS~1\Temp\RSP94D.tmp" with contents
[
/nologo /W3 /I "..\OpenTimeCE\\" /I "..\SDL-1.2.12\include" /I "..\ffmpeg\include" /I "..\ffmpeg" /I "..\ffmpeg\libavcodec" /I "..\ffmpeg\libavformat" /I "..\ffmpeg\libavutil" /D _WIN32_WCE=500 /D "ARM" /D "_ARM_" /D "WCE_PLATFORM_Mantas500B" /D "ARMV4I" /D UNDER_CE=500 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_LIB" /D "FFPLAY_LIB" /Fp"..\..\..\Build\Intermediate\ffplaylibce\ARMV4IRelease\FFPlayLibCE.pch" /YX /Fo"..\..\..\Build\Intermediate\ffplaylibce\ARMV4IRelease\\" /QRarch4T /QRinterwork-return /O2 /MC /c 
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\FFMPEG\cmdutils.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\FFMPEG\misc.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\FFMPEG\ffplayLib.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\ms\LOCALS~1\Temp\RSP94D.tmp" 
Creating temporary file "C:\DOCUME~1\ms\LOCALS~1\Temp\RSP94E.tmp" with contents
[
/nologo /out:"..\..\..\Build\Lib\libavcodec\ARMV4I\FFPlayLibCE.lib" 
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\ffplaylibce\ARMV4IRelease\cmdutils.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\ffplaylibce\ARMV4IRelease\misc.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\ffplaylibce\ARMV4IRelease\ffplayLib.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Lib\libavcodec\ARMV4I\libavcodec.lib"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Lib\libavformat\ARMV4I\libavformat.lib"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Lib\libavutil\ARMV4I\libavutil.lib"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Lib\libswscale\ARMV4I\libswscale.lib"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\lib\Libvorbis\ARMV4I\libvorbisidec.lib"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\lib\OpenTimeCE\ARMV4I\OpenTimeCE.lib"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\SDL\ARMV4IRelease\SDL.lib"
]
Creating command line "link.exe -lib @C:\DOCUME~1\ms\LOCALS~1\Temp\RSP94E.tmp"
<h3>Output Window</h3>
Compiling...
cmdutils.c
misc.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\misc.c(7) : warning C4244: 'return' : conversion from 'int ' to 'float ', possible loss of data
ffplayLib.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\sdl-1.2.12\include\sdl_config_win32.h(33) : warning C4142: benign redefinition of type
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\sdl-1.2.12\include\sdl_config_win32.h(34) : warning C4142: benign redefinition of type
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\sdl-1.2.12\include\sdl_config_win32.h(35) : warning C4142: benign redefinition of type
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\sdl-1.2.12\include\sdl_config_win32.h(36) : warning C4142: benign redefinition of type
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\sdl-1.2.12\include\sdl_config_win32.h(37) : warning C4142: benign redefinition of type
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\sdl-1.2.12\include\sdl_config_win32.h(38) : warning C4142: benign redefinition of type
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(448) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(449) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(450) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(461) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(467) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(468) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(469) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(477) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(478) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(479) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(496) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(503) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(504) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(505) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(516) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(522) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(530) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(536) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(538) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(539) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(551) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(558) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(559) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(560) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(579) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(580) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(581) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(592) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(598) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(599) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(600) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(608) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(609) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(610) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(619) : warning C4018: '<' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(647) : warning C4244: '=' : conversion from 'double ' to 'float ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(673) : warning C4018: '<' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(684) : warning C4013: 'rint' undefined; assuming extern returning int
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(737) : warning C4244: '=' : conversion from '__int64 ' to 'short ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1331) : warning C4244: '=' : conversion from 'unsigned __int64 ' to 'double ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1333) : warning C4244: '=' : conversion from '__int64 ' to 'double ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1401) : warning C4018: '<' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1613) : warning C4018: '>=' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1646) : warning C4018: '>=' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1738) : warning C4018: '>=' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(1911) : warning C4018: '<' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2132) : warning C4018: '>=' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2279) : warning C4013: 'ffplay_seek' undefined; assuming extern returning int
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2294) : warning C4244: '=' : conversion from '__int64 ' to 'double ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2300) : warning C4244: 'function' : conversion from 'double ' to '__int64 ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2300) : warning C4244: 'function' : conversion from 'double ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2304) : warning C4244: 'function' : conversion from 'double ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2317) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2325) : warning C4244: '=' : conversion from 'double ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2617) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2620) : warning C4244: '=' : conversion from 'double ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2781) : warning C4244: 'function' : conversion from 'double ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(3100) : warning C4018: '<' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2832) : warning C4715: 'ffplay_reset' : not all control paths return a value
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\ffplaylib.c(2909) : warning C4716: 'ffplay_set_volume' : must return a value
Creating library...




<h3>Results</h3>
FFPlayLibCE.lib - 0 error(s), 67 warning(s)
</pre>
</body>
</html>
