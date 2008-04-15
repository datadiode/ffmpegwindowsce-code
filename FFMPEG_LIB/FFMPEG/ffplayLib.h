#ifndef __FFPLAY_TEST_H
#define __FFPLAY_TEST_H
#include "SDL.h"
#ifdef __cplusplus 
extern "C"{
#endif

//Player interface 
typedef enum {
	FFPLAY_OK=0,
	FFPLAY_ERROR
} enFFPlayResult; 

typedef struct _ffplayStreamInfo{ 
    char title[512];
    char author[512];
    char copyright[512];
    char comment[512];
    char album[512];
    int year;  /**< ID3 year, 0 if none */
    int track; /**< track number, 0 if none */
    char genre[32]; /**< ID3 genre */
}stFFPlayStreamInfo; 

typedef struct _ffplayStreamStaus{
	int isplaying; // 1= stream is proceed / 0 = stream is not procesed
	int nPos;// current stream cursor position
	int nLen;// total stream length
} stFFPlayStreamStatus; 



//Init the use of FFPLAY library 
enFFPlayResult ffplay_init(); 

//Cleanup the use of FFPLAY library 
void ffplay_deinit();

// Open a stream for palyback
enFFPlayResult ffplay_open(const char* szFileName);

//Start playback of the current opened stream
enFFPlayResult ffplay_play();

//Stop playback of the current opened stream
enFFPlayResult ffplay_stop();

//Pause playback of the current opened stream.
enFFPlayResult ffplay_pause();

//Resume playback of the current paused opened stream.
enFFPlayResult ffplay_resume();

//Pause playback of the current opened stream and seek to beginig of the stream
enFFPlayResult ffplay_reset();

//Skip playback of the current opened stream
// nPos in bytes.
enFFPlayResult ffplay_seek_byte(int nPos);


//Skip playback of the current opened stream
// nPos in seconds from the begining of the stream
enFFPlayResult ffplay_seek_sec(int nPos);


//Skip playback of the current opened stream
// nPos in seconds can take -/+ values to skip backward/forward
enFFPlayResult ffplay_seek_sec_dif(int nPos);


//Read stream information. 
// This function returns information about szFileName 
// szFileName - full path of the input file 
// pInfo - pointer to a stFFPlayStreamInfo structure that will return the result information
enFFPlayResult ffplay_read_info(const char* szFileName,stFFPlayStreamInfo* pInfo);

//Read currentopened stream status 
enFFPlayResult ffplay_get_status(stFFPlayStreamStatus* pStatus);

//Set current stream volume in percent 
enFFPlayResult ffplay_set_volume(int nProcent); 

SDL_Thread* GetCurrentParserThread();


#ifdef __cplusplus
}
#endif

#endif 
