<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: libavutil - Win32 (WCE ARMV4I) Release Fixpoint--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FA.tmp" with contents
[
/nologo /W3 /I "..\ffmpeg\include" /I "..\ffmpeg\libavutil" /I "..\ffmpeg\libavcodec" /D _WIN32_WCE=500 /D "ARM" /D "_ARM_" /D "WCE_PLATFORM_Mantas500B" /D "ARMV4I" /D UNDER_CE=500 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "MS99_TEST" /D "USE_HIGHPRECISION" /D "CONFIG_ALIGN" /D "UNDER_CE" /Fp"..\..\..\Build\Intermediate\libavutil\ARMV4IRelease\libavutil.pch" /YX /Fo"..\..\..\Build\Intermediate\libavutil\ARMV4IRelease\\" /QRarch4T /QRinterwork-return /O2 /MC /c 
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\adler32.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\aes.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\base64.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\crc.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\fifo.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\integer.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\intfloat_readwrite.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\lls.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\log.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\lzo.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\mathematics.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\md5.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\mem.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\random.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\rational.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\sha1.c"
"D:\smu_smu-arobs-laptop_mn7_Hera\Projects\HeraMultimedia\FFMPEG_LIB\ffmpeg\libavutil\tree.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FA.tmp" 
Creating temporary file "C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FB.tmp" with contents
[
/nologo /out:"..\..\..\Build\Lib\libavutil\ARMV4I\libavutil.lib" 
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\adler32.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\aes.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\base64.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\crc.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\fifo.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\integer.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\intfloat_readwrite.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\lls.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\log.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\lzo.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\mathematics.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\md5.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\mem.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\random.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\rational.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\sha1.obj"
"\smu_smu-arobs-laptop_mn7_Hera\Projects\Build\Intermediate\libavutil\ARMV4IRelease\tree.obj"
]
Creating command line "link.exe -lib @C:\DOCUME~1\ms\LOCALS~1\Temp\RSP8FB.tmp"
<h3>Output Window</h3>
Compiling...
adler32.c
aes.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(54) : warning C4047: 'initializing' : 'unsigned char (*)[16]' differs in levels of indirection from 'unsigned char *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(55) : warning C4047: 'initializing' : 'unsigned char (*)[16]' differs in levels of indirection from 'unsigned char *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(82) : warning C4047: 'function' : 'unsigned long (*)[256]' differs in levels of indirection from 'unsigned long *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(82) : warning C4024: 'mix' : different types for formal and actual parameter 2
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(83) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(83) : warning C4024: 'addkey' : different types for formal and actual parameter 1
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(83) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(83) : warning C4024: 'addkey' : different types for formal and actual parameter 2
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(83) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(83) : warning C4024: 'addkey' : different types for formal and actual parameter 3
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(85) : warning C4047: 'function' : 'unsigned char (*)[16]' differs in levels of indirection from 'unsigned char [4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(85) : warning C4024: 'subshift' : different types for formal and actual parameter 1
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(90) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(90) : warning C4024: 'addkey' : different types for formal and actual parameter 1
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(90) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned __int64 *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(90) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(90) : warning C4024: 'addkey' : different types for formal and actual parameter 3
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(92) : warning C4047: 'function' : 'unsigned long *' differs in levels of indirection from 'unsigned long [4][256]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(92) : warning C4024: 'crypt' : different types for formal and actual parameter 4
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(94) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(94) : warning C4024: 'addkey' : different types for formal and actual parameter 1
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(94) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(94) : warning C4024: 'addkey' : different types for formal and actual parameter 2
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(94) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned __int64 *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(97) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned __int64 *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(97) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(97) : warning C4024: 'addkey' : different types for formal and actual parameter 2
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(97) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(97) : warning C4024: 'addkey' : different types for formal and actual parameter 3
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(99) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(99) : warning C4024: 'addkey' : different types for formal and actual parameter 1
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(99) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(99) : warning C4024: 'addkey' : different types for formal and actual parameter 2
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(99) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned __int64 *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(100) : warning C4047: 'function' : 'unsigned long *' differs in levels of indirection from 'unsigned long [4][256]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(100) : warning C4024: 'crypt' : different types for formal and actual parameter 4
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(101) : warning C4133: 'function' : incompatible types - from 'unsigned char *' to 'unsigned __int64 *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(101) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(101) : warning C4024: 'addkey' : different types for formal and actual parameter 2
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(101) : warning C4047: 'function' : 'unsigned __int64 *' differs in levels of indirection from 'unsigned char [4][4]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(101) : warning C4024: 'addkey' : different types for formal and actual parameter 3
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(150) : warning C4133: 'function' : incompatible types - from 'unsigned long [256]' to 'unsigned char *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(151) : warning C4133: 'function' : incompatible types - from 'unsigned long [256]' to 'unsigned char *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(181) : warning C4047: 'function' : 'unsigned char (*)[16]' differs in levels of indirection from 'unsigned char [16]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(181) : warning C4024: 'subshift' : different types for formal and actual parameter 1
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(182) : warning C4048: different array subscripts : 'unsigned char (*)[4][4]' and 'unsigned char [3][16]'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\aes.c(182) : warning C4024: 'mix' : different types for formal and actual parameter 1
base64.c
crc.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\crc.c(59) : warning C4146: unary minus operator applied to unsigned type, result still unsigned
fifo.c
integer.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\integer.c(144) : warning C4244: '=' : conversion from '__int64 ' to 'unsigned short ', possible loss of data
intfloat_readwrite.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(53) : warning C4244: 'function' : conversion from '__int64 ' to 'double ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(53) : warning C4244: 'function' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(70) : warning C4244: 'return' : conversion from 'double ' to 'float ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(73) : warning C4244: 'return' : conversion from 'double ' to 'float ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(100) : warning C4146: unary minus operator applied to unsigned type, result still unsigned
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(101) : warning C4244: 'function' : conversion from 'unsigned __int64 ' to 'double ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(124) : warning C4244: '=' : conversion from 'double ' to 'float ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(125) : warning C4244: 'return' : conversion from '__int64 ' to 'long ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\intfloat_readwrite.c(139) : warning C4244: '=' : conversion from 'unsigned __int64 ' to 'unsigned char ', possible loss of data
lls.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\lls.c(56) : warning C4047: 'initializing' : 'double (*)[33]' differs in levels of indirection from 'double *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\lls.c(57) : warning C4047: 'initializing' : 'double (*)[33]' differs in levels of indirection from 'double *'
log.c
lzo.c
mathematics.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\internal.h(297) : warning C4013: 'rint' undefined; assuming extern returning int
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\mathematics.c(83) : warning C4018: '<' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\mathematics.c(89) : warning C4018: '<=' : signed/unsigned mismatch
md5.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\md5.c(90) : warning C4101: 'i' : unreferenced local variable
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\md5.c(130) : warning C4244: '=' : conversion from 'unsigned __int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\md5.c(144) : warning C4013: 'le2me_64' undefined; assuming extern returning int
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\md5.c(150) : warning C4133: 'function' : incompatible types - from 'unsigned __int64 *' to 'const unsigned char *'
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\md5.c(153) : warning C4013: 'le2me_32' undefined; assuming extern returning int
mem.c
random.c
rational.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(47) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(48) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(60) : warning C4018: '>' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(62) : warning C4018: '>' : signed/unsigned mismatch
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(64) : warning C4244: '=' : conversion from 'unsigned __int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(65) : warning C4244: '=' : conversion from 'unsigned __int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(71) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\rational.c(72) : warning C4244: '=' : conversion from '__int64 ' to 'int ', possible loss of data
sha1.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\sha1.c(92) : warning C4244: '=' : conversion from 'unsigned __int64 ' to 'unsigned int ', possible loss of data
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\sha1.c(124) : warning C4133: 'function' : incompatible types - from 'unsigned __int64 *' to 'unsigned char *'
tree.c
d:\smu_smu-arobs-laptop_mn7_hera\projects\heramultimedia\ffmpeg_lib\ffmpeg\libavutil\tree.c(92) : warning C4717: 'av_tree_destroy' : recursive on all control paths, function will cause runtime stack overflow
Creating library...




<h3>Results</h3>
libavutil.lib - 0 error(s), 79 warning(s)
</pre>
</body>
</html>
