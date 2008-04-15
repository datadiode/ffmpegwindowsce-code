<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: libswscale - Win32 (WCE ARMV4I) Release Fixpoint--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FE.tmp" with contents
[
/nologo /W3 /I "..\ffmpeg\include" /I "..\ffmpeg\libavutil" /I "..\ffmpeg\libavcodec" /D _WIN32_WCE=500 /D "ARM" /D "_ARM_" /D "WCE_PLATFORM_Mantas500B" /D "ARMV4I" /D UNDER_CE=500 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "MS99_TEST" /D "USE_HIGHPRECISION" /D "CONFIG_ALIGN" /D "UNDER_CE" /Fp"..\..\..\Build\Intermediate\libswscale\ARMV4IRelease\libswscale.pch" /YX /Fo"..\..\..\Build\Intermediate\libswscale\ARMV4IRelease\\" /QRarch4T /QRinterwork-return /O2 /MC /c 
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libswscale\rgb2rgb.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libswscale\swscale.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libswscale\yuv2rgb.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FE.tmp" 
Creating temporary file "C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FF.tmp" with contents
[
/nologo /out:"..\..\..\Build\Lib\libswscale\ARMV4I\libswscale.lib" 
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libswscale\ARMV4IRelease\rgb2rgb.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libswscale\ARMV4IRelease\swscale.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libswscale\ARMV4IRelease\yuv2rgb.obj"
]
Creating command line "link.exe -lib @C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FF.tmp"
<h3>Output Window</h3>
Compiling...
rgb2rgb.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\rgb2rgb_template.c(1491) : warning C4018: '<' : signed/unsigned mismatch
swscale.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(855) : warning C4101: 'b' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(855) : warning C4101: 'r' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(855) : warning C4101: 'g' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(863) : warning C4101: 'b' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(863) : warning C4101: 'r' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(863) : warning C4101: 'g' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(1834) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(1937) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(2158) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(2190) : warning C4244: '=' : conversion from 'long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(2282) : warning C4244: '=' : conversion from 'unsigned long ' to 'unsigned char ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(2541) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned long *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale_template.c(2757) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned long *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(1341) : warning C4244: 'initializing' : conversion from 'double ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(1573) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(1861) : warning C4133: 'initializing' : incompatible types - from 'unsigned char *' to 'unsigned short *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(1862) : warning C4133: 'initializing' : incompatible types - from 'unsigned char *' to 'unsigned short *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(1913) : warning C4244: 'initializing' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\swscale.c(2554) : warning C4133: '=' : incompatible types - from 'unsigned long *' to 'unsigned char *'
yuv2rgb.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(560) : warning C4101: 'b' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(560) : warning C4101: 'r' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(560) : warning C4101: 'U' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(560) : warning C4101: 'V' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(639) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(640) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(641) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(645) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(647) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(649) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(651) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(652) : warning C4550: expression evaluates to a function which is missing an argument list
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(712) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(863) : warning C4244: 'function' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(864) : warning C4244: 'function' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(865) : warning C4244: 'function' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libswscale\yuv2rgb.c(866) : warning C4244: 'function' : conversion from '__int64 ' to 'int ', possible loss of data
Creating library...




<h3>Results</h3>
libswscale.lib - 0 error(s), 37 warning(s)
</pre>
</body>
</html>
