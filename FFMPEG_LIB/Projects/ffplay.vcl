<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: ffplay - Win32 (WCE ARMV4I) Debug--------------------
</h3>
<h3>Command Lines</h3>
Build : warning : failed to (or don't know how to) build 'D:\multimedia\FFMPEG_LIB\ffmpeg\ffplayTest.c'

Creating temporary file "C:\DOCUME~1\MiCos\LOCALS~1\Temp\RSPC6.tmp" with contents
[
/nologo /W3 /Zi /Od /I "..\OpenTimeCE\\" /I "..\SDL-1.2.7\include" /I "..\ffmpeg\include" /I "..\ffmpeg" /I "..\ffmpeg\libavcodec" /I "..\ffmpeg\libavformat" /I "..\ffmpeg\libavutil" /D "DEBUG" /D "ARM" /D "_ARM_" /D "ARMV4I" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WCE_PLATFORM_STANDARDSDK" /D "UNICODE" /D "_UNICODE" /FR"ffplay\ARMV4IDbg/" /Fp"ffplay\ARMV4IDbg/ffplay.pch" /YX /Fo"ffplay\ARMV4IDbg/" /Fd"ffplay\ARMV4IDbg/" /QRarch4T /QRinterwork-return /MC /c 
"D:\multimedia\FFMPEG_LIB\ffmpeg\ffplayTest.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\MiCos\LOCALS~1\Temp\RSPC6.tmp" 
Creating temporary file "C:\DOCUME~1\MiCos\LOCALS~1\Temp\RSPC7.tmp" with contents
[
Ws2.lib commctrl.lib coredll.lib corelibc.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:yes /pdb:"ffplay\ARMV4IDbg/ffplay.pdb" /debug /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib" /out:"ffplay\ARMV4IDbg/ffplay.exe" /subsystem:windowsce,4.20 /MACHINE:THUMB 
.\ffplay\ARMV4IDbg\cmdutils.obj
.\ffplay\ARMV4IDbg\ffplayTest.obj
.\ffplay\ARMV4IDbg\misc.obj
]
Creating command line "link.exe @C:\DOCUME~1\MiCos\LOCALS~1\Temp\RSPC7.tmp"
<h3>Output Window</h3>
Compiling...
ffplayTest.c
fatal error C1083: Cannot open source file: 'D:\multimedia\FFMPEG_LIB\ffmpeg\ffplayTest.c': No such file or directory
Error executing clarm.exe.



<h3>Results</h3>
ffplay.exe - 1 error(s), 1 warning(s)
</pre>
</body>
</html>
