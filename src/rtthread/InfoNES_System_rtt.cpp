/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_Linux.cpp : Linux specific File                   */
/*                                                                   */
/*  2001/05/18  InfoNES Project ( Sound is based on DarcNES )        */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_posix.h>

#include "../InfoNES.h"
#include "../InfoNES_System.h"
#include "../InfoNES_pAPU.h"

#define NES_ROM_NAME	"/dyn/preset_user/nes/SuperMarioBros.nes"

/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/

char szRomName[ 256 ];
char szSaveName[ 256 ];
int nSRAM_SaveFlag;

/*-------------------------------------------------------------------*/
/*  Constants ( Linux specific )                                     */
/*-------------------------------------------------------------------*/

#define VBOX_SIZE    7
#define SOUND_DEVICE "/dev/dsp"
#define VERSION      "InfoNES v0.96J"

/*-------------------------------------------------------------------*/
/*  Global Variables ( rt-thread specific )                              */
/*-------------------------------------------------------------------*/
rt_thread_t g_nes_tid;

/* Pad state */
//DWORD dwKeyPad1;
DWORD dwKeyPad2;
DWORD dwKeySystem;

/* For Sound Emulation */
BYTE final_wave[2048];
int waveptr;
int wavflag;
int sound_fd;

#ifdef __cplusplus
extern "C" {
#endif
/* Pad state */
extern DWORD dwKeyPad1;
/* lv canvas for display */
extern WORD canvas_buffer[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
extern void nes_canvas_refresh(void);
/* audio */
extern void infoNES_audio_init(void);
extern int infoNES_audio_open(int samples_per_sync, int sample_rate);
extern void InfoNES_audio_close( void );
extern int InfoNES_audio_write(uint8_t *data, uint32_t data_len);
/* for c api */
void start_application( void );
void close_application( void );
#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Linux specific )                           */
/*-------------------------------------------------------------------*/

int LoadSRAM();
int SaveSRAM();

/* Palette data */
WORD NesPalette[ 64 ] =
{
  0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
  0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
  0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
  0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
  0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
  0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
};

#if 0
/*===================================================================*/
/*                                                                   */
/*                main() : Application main                          */
/*                                                                   */
/*===================================================================*/

/* Application main */
int main(int argc, char **argv)
{
  int       i;
#if 0
  /*-------------------------------------------------------------------*/
  /*  Initialize GTK+/GDK                                              */
  /*-------------------------------------------------------------------*/

  g_thread_init( NULL );
  gtk_set_locale();
  gtk_init(&argc, &argv);
  gdk_rgb_init();

  /*-------------------------------------------------------------------*/
  /*  Create a top window                                              */
  /*-------------------------------------------------------------------*/

  /* Create a window */
  top=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize( GTK_WIDGET(top), NES_DISP_WIDTH + VBOX_SIZE, NES_DISP_HEIGHT + VBOX_SIZE );
  gtk_window_set_title( GTK_WINDOW(top), VERSION );
  gtk_widget_set_events( GTK_WIDGET(top), GDK_KEY_RELEASE_MASK );

  /* Destroy a window */
  gtk_signal_connect( GTK_OBJECT(top), "destroy",
		      GTK_SIGNAL_FUNC( close_application ),
		      NULL );

  /* Create a vbox */
  vbox=gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 3);
  gtk_container_add(GTK_CONTAINER(top), vbox);

  /* Create a drawing area */
  draw_area = gtk_drawing_area_new();
  gtk_box_pack_start(GTK_BOX (vbox), draw_area, TRUE, TRUE, 0);

  gtk_widget_show_all(top);
#endif

  /*-------------------------------------------------------------------*/
  /*  Pad Control                                                      */
  /*-------------------------------------------------------------------*/

  /* Initialize a pad state */
  dwKeyPad1   = 0;
  dwKeyPad2   = 0;
  dwKeySystem = 0;

#if 0
  /* Connecting to key event */
  gtk_signal_connect( GTK_OBJECT(top), "key_press_event",
		      GTK_SIGNAL_FUNC( add_key ),
		      NULL );

  gtk_signal_connect( GTK_OBJECT(top), "key_release_event",
		      GTK_SIGNAL_FUNC( remove_key ),
		      NULL );
#endif

  /*-------------------------------------------------------------------*/
  /*  Load Cassette & Create Thread                                    */
  /*-------------------------------------------------------------------*/

#if 0
  /* Initialize thread state */
  bThread = FALSE;

  /* If a rom name specified, start it */
  if ( argc == 2 )
  {
    start_application( argv[ 1 ] );
  }

  /* show the window */
  gdk_threads_enter();
  gtk_main ();
  gdk_threads_leave();
#endif

  return(0);
}
#endif

/*===================================================================*/
/*                                                                   */
/*           emulation_thread() : Thread Hooking Routine             */
/*                                                                   */
/*===================================================================*/

void emulation_thread(void *args)
{
  InfoNES_Main();
  return;
}

/*===================================================================*/
/*                                                                   */
/*     start_application() : Start NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void start_application( void )
{
  /* Set a ROM image name */
  strcpy( szRomName, NES_ROM_NAME );

  /* Load cassette */
  if ( InfoNES_Load ( szRomName ) == 0 )
  {
    rt_kprintf( "Load %s OK\n", szRomName );
    /* Load SRAM */
    LoadSRAM();
    rt_kprintf( "Load SRAM done\n" );

    /* Create Emulation Thread */
    g_nes_tid = rt_thread_create("nes_emu",
                           emulation_thread, RT_NULL,
                           4096, RT_THREAD_PRIORITY_LOW, RT_THREAD_TICK_DEFAULT);

    if (g_nes_tid != RT_NULL)
        rt_thread_startup(g_nes_tid);
  }

  FrameSkip = 2;
}

/*===================================================================*/
/*                                                                   */
/*     close_application() : When invoked via signal delete_event    */
/*                                                                   */
/*===================================================================*/
void close_application( void )
{
  /* Save SRAM*/
  SaveSRAM();

  if ( g_nes_tid != RT_NULL )
  {
    rt_thread_delete( g_nes_tid );
    g_nes_tid = RT_NULL;
  }
}

/*===================================================================*/
/*                                                                   */
/*     reset_application() : Reset NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void reset_application( void )
{
  /* Do nothing if emulation thread does not exists */
  if ( g_nes_tid != RT_NULL )
  {
    /* Terminate emulation thread */
    dwKeySystem |= PAD_SYS_QUIT;

    close_application();
    start_application();
  }
}

/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int LoadSRAM()
{
/*
 *  Load a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be read
 */

  int fp;
  static unsigned char pSrcBuf[ SRAM_SIZE ];
  unsigned char chData;
  unsigned char chTag;
  int nRunLen;
  int nDecoded;
  int nDecLen;
  int nIdx;

  // It doesn't need to save it
  nSRAM_SaveFlag = 0;

  // It is finished if the ROM doesn't have SRAM
  if ( !ROM_SRAM )
    return 0;

  // There is necessity to save it
  nSRAM_SaveFlag = 1;

  // The preparation of the SRAM file name
  strcpy( szSaveName, szRomName );
  strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );

  /*-------------------------------------------------------------------*/
  /*  Read a SRAM data                                                 */
  /*-------------------------------------------------------------------*/

  // Open SRAM file
  fp = open( szSaveName, O_RDONLY | O_BINARY );
  if ( fp < 0 )
    return -1;

  // Read SRAM data
  read( fp, pSrcBuf, SRAM_SIZE );

  // Close SRAM file
  close( fp );

  /*-------------------------------------------------------------------*/
  /*  Extract a SRAM data                                              */
  /*-------------------------------------------------------------------*/

  nDecoded = 0;
  nDecLen = 0;

  chTag = pSrcBuf[ nDecoded++ ];

  while ( nDecLen < 8192 )
  {
    chData = pSrcBuf[ nDecoded++ ];

    if ( chData == chTag )
    {
      chData = pSrcBuf[ nDecoded++ ];
      nRunLen = pSrcBuf[ nDecoded++ ];
      for ( nIdx = 0; nIdx < nRunLen + 1; ++nIdx )
      {
        SRAM[ nDecLen++ ] = chData;
      }
    }
    else
    {
      SRAM[ nDecLen++ ] = chData;
    }
  }

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
int SaveSRAM()
{
/*
 *  Save a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be written
 */

  int fp;
  int nUsedTable[ 256 ];
  unsigned char chData;
  unsigned char chPrevData;
  unsigned char chTag;
  int nIdx;
  int nEncoded;
  int nEncLen;
  int nRunLen;
  static unsigned char pDstBuf[ SRAM_SIZE ];

  if ( !nSRAM_SaveFlag )
    return 0;  // It doesn't need to save it

  /*-------------------------------------------------------------------*/
  /*  Compress a SRAM data                                             */
  /*-------------------------------------------------------------------*/

  memset( nUsedTable, 0, sizeof nUsedTable );

  for ( nIdx = 0; nIdx < SRAM_SIZE; ++nIdx )
  {
    ++nUsedTable[ SRAM[ nIdx++ ] ];
  }
  for ( nIdx = 1, chTag = 0; nIdx < 256; ++nIdx )
  {
    if ( nUsedTable[ nIdx ] < nUsedTable[ chTag ] )
      chTag = nIdx;
  }

  nEncoded = 0;
  nEncLen = 0;
  nRunLen = 1;

  pDstBuf[ nEncLen++ ] = chTag;

  chPrevData = SRAM[ nEncoded++ ];

  while ( nEncoded < SRAM_SIZE && nEncLen < SRAM_SIZE - 133 )
  {
    chData = SRAM[ nEncoded++ ];

    if ( chPrevData == chData && nRunLen < 256 )
      ++nRunLen;
    else
    {
      if ( nRunLen >= 4 || chPrevData == chTag )
      {
        pDstBuf[ nEncLen++ ] = chTag;
        pDstBuf[ nEncLen++ ] = chPrevData;
        pDstBuf[ nEncLen++ ] = nRunLen - 1;
      }
      else
      {
        for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
          pDstBuf[ nEncLen++ ] = chPrevData;
      }

      chPrevData = chData;
      nRunLen = 1;
    }

  }
  if ( nRunLen >= 4 || chPrevData == chTag )
  {
    pDstBuf[ nEncLen++ ] = chTag;
    pDstBuf[ nEncLen++ ] = chPrevData;
    pDstBuf[ nEncLen++ ] = nRunLen - 1;
  }
  else
  {
    for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
      pDstBuf[ nEncLen++ ] = chPrevData;
  }

  /*-------------------------------------------------------------------*/
  /*  Write a SRAM data                                                */
  /*-------------------------------------------------------------------*/

  // Open SRAM file
  fp = open( szSaveName, O_WRONLY | O_BINARY );
  if ( fp < 0 )
    return -1;

  // Write SRAM data
  write( fp, pDstBuf, nEncLen );

  // Close SRAM file
  close( fp );

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

  /* If terminated */
  if ( g_nes_tid == NULL )
  {
    return -1;
  }

  /* Nothing to do here */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

  int fp;

  /* Open ROM file */
  fp = open( pszFileName, O_RDONLY | O_BINARY );
  if ( fp < 0 )
    return -1;

  /* Read ROM Header */
  read( fp, &NesHeader, sizeof(NesHeader) );
  if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
  {
    /* not .nes file */
    close( fp );
    return -1;
  }

  /* Clear SRAM */
  memset( SRAM, 0, SRAM_SIZE );

  /* If trainer presents Read Triner at 0x7000-0x71ff */
  if ( NesHeader.byInfo1 & 4 )
  {
    read( fp, &SRAM[ 0x1000 ], 512 );
  }

  /* Allocate Memory for ROM Image */
  ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );
  if ( ROM == NULL )
  {
    close( fp );
    return -1;
  }

  /* Read ROM Image */
  read( fp, ROM, NesHeader.byRomSize * 0x4000 );

  if ( NesHeader.byVRomSize > 0 )
  {
    /* Allocate Memory for VROM Image */
    VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

    /* Read VROM Image */
    read( fp, VROM, NesHeader.byVRomSize * 0x2000 );
  }

  /* File close */
  close( fp );

  /* Successful */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */

  if ( ROM )
  {
    free( ROM );
    ROM = NULL;
  }

  if ( VROM )
  {
    free( VROM );
    VROM = NULL;
  }
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block's destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

  memcpy( dest, src, count );
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
/*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

  memset( dest, c, count);
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{
  for ( int y = 0; y < NES_DISP_HEIGHT; y++ )
  {
    for ( int x = 0; x < NES_DISP_WIDTH; x++ )
    {
      WORD wColor = WorkFrame[ ( y << 8 ) + x ];
      /* RGB555 to RGB565 */
      canvas_buffer[ ( y << 8 ) + x ] = (wColor & 0x7c00) << 1  //R: keep format, move position
      | (wColor & 0x03e0) << 1 | (wColor & 0x0200) >> 4 //G: 5 bits expend to 6 bits, fill bit0 with bit4
      | (wColor & 0x001f) ; //B: keep format
    }
  }
  nes_canvas_refresh();
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
/*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for InfoNES
 *
 */

  /* Transfer joypad state */
  *pdwPad1   = dwKeyPad1;
  *pdwPad2   = dwKeyPad2;
  *pdwSystem = dwKeySystem;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void )
{
  infoNES_audio_init();
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate )
{
  if (infoNES_audio_open (samples_per_sync, sample_rate) < 0)
  {
    return 0;
  }

  /* Successful */
  return 1;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void )
{
  InfoNES_audio_close();
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 )
{
  int i;

  //if ( sound_fd )
  {
    for (i = 0; i < samples; i++)
    {
#if 1
      final_wave[ waveptr ] =
	( wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i] ) / 5;
#else
      final_wave[ waveptr ] = wave4[i];
#endif


      waveptr++;
      if ( waveptr == 2048 )
      {
	waveptr = 0;
	wavflag = 2;
      }
      else if ( waveptr == 1024 )
      {
	wavflag = 1;
      }
    }

    if ( wavflag )
    {
      if ( InfoNES_audio_write( &final_wave[(wavflag - 1) << 10], 1024) < 1024 )
      {
	InfoNES_MessageBox( "wrote less than 1024 bytes\n" );
      }
      wavflag = 0;
    }
  }
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait() {
  //rt_thread_mdelay(1);
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( const char *pszMsg, ... )
{
  char pszErr[ 1024 ];
  va_list args;

  // Create the message body
  va_start( args, pszMsg );
  vsprintf( pszErr, pszMsg, args );  pszErr[ 1023 ] = '\0';
  va_end( args );

  rt_kprintf( "%s", pszErr );
}

#if 0
void close_dialog( GtkWidget *widget, gpointer data )
{
  gtk_widget_destroy( GTK_WIDGET( data ) );
}

void closing_dialog( GtkWidget *widget, gpointer data )
{
  gtk_grab_remove( GTK_WIDGET( widget ) );
}
#endif

/*
 * End of InfoNES_System_rtt.cpp
 */
