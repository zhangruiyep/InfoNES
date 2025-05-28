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
rt_thread_t tid;

/* Pad state */
DWORD dwKeyPad1;
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
extern WORD canvas_buffer[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
extern void nes_canvas_refresh(void);
void start_application( void );
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
/*          add_key() : Connecting to the key_press_event event      */
/*                                                                   */
/*===================================================================*/
#if 0
void add_key( GtkWidget *widget, GdkEventKey *event, gpointer callback_data )
{
  switch ( event->keyval )
  {
    case GDK_Right:
      dwKeyPad1 |= ( 1 << 7 );
      break;

    case GDK_Left:
      dwKeyPad1 |= ( 1 << 6 );
      break;

    case GDK_Down:
      dwKeyPad1 |= ( 1 << 5 );
      break;

    case GDK_Up:
      dwKeyPad1 |= ( 1 << 4 );
      break;

    case 's':
    case 'S':
      /* Start */
      dwKeyPad1 |= ( 1 << 3 );
      break;

    case 'a':
    case 'A':
      /* Select */
      dwKeyPad1 |= ( 1 << 2 );
      break;

    case 'z':
    case 'Z':
      /* 'A' */
      dwKeyPad1 |= ( 1 << 1 );
      break;

    case 'x':
    case 'X':
      /* 'B' */
      dwKeyPad1 |= ( 1 << 0 );
      break;

    case 'c':
    case 'C':
      /* Toggle up and down clipping */
      PPU_UpDown_Clip = ( PPU_UpDown_Clip ? 0 : 1 );
      break;

    case 'q':
    case 'Q':
      close_application( widget, NULL, NULL );
      break;

    case 'r':
    case 'R':
      /* Reset the application */
      reset_application();
      break;

    case 'l':
    case 'L':
      /* If emulation thread runs, nothing here */
      if ( bThread != TRUE )
      {
	/* Create a file selection widget */
	filew = gtk_file_selection_new( "Load" );
	gtk_widget_show( filew );

        /* Connecting to button event */
	gtk_signal_connect_object( GTK_OBJECT( GTK_FILE_SELECTION( filew )->ok_button ),
				   "clicked",
				   ( GtkSignalFunc ) start_application_aux,
				   GTK_OBJECT( filew ) );

	gtk_signal_connect_object( GTK_OBJECT( GTK_FILE_SELECTION( filew )->cancel_button ),
				   "clicked",
				   ( GtkSignalFunc ) gtk_widget_destroy,
				   GTK_OBJECT( filew ) );
      }
      break;

    case GDK_Page_Up:
      /* Increase Frame Skip */
      FrameSkip++;
      break;

    case GDK_Page_Down:
      /* Decrease Frame Skip */
      if ( FrameSkip > 0 )
      {
	FrameSkip--;
      }
      break;

    case 'm':
    case 'M':
      /* Toggle of sound mute */
      APU_Mute = ( APU_Mute ? 0 : 1 );
      break;

    case 'i':
    case 'I':
      /* If emulation thread doesn't run, nothing here */
      if ( bThread )
      {
	InfoNES_MessageBox( "Mapper : %d\nPRG ROM : %dKB\nCHR ROM : %dKB\n" \
			    "Mirroring : %s\nSRAM : %s\n4 Screen : %s\nTrainer : %s\n",
			    MapperNo, NesHeader.byRomSize * 16, NesHeader.byVRomSize * 8,
			    ( ROM_Mirroring ? "V" : "H" ), ( ROM_SRAM ? "Yes" : "No" ),
			    ( ROM_FourScr ? "Yes" : "No" ), ( ROM_Trainer ? "Yes" : "No" ) );
      }
      break;

    case 'v':
    case 'V':
      /* Version Infomation */
      InfoNES_MessageBox( "%s\nA fast and portable NES emulator\n"
			  "Copyright (c) 1999-2005 Jay's Factory <jays_factory@excite.co.jp>",
			  VERSION );
      break;

    defalut:
      break;
  }
}

/*===================================================================*/
/*                                                                   */
/*       remove_key() : Connecting to the key_release_event event    */
/*                                                                   */
/*===================================================================*/

void remove_key( GtkWidget *widget, GdkEventKey *event, gpointer callback_data )
{
  switch ( event->keyval )
  {
    case GDK_Right:
      dwKeyPad1 &= ~( 1 << 7 );
      break;

    case GDK_Left:
      dwKeyPad1 &= ~( 1 << 6 );
      break;

    case GDK_Down:
      dwKeyPad1 &= ~( 1 << 5 );
      break;

    case GDK_Up:
      dwKeyPad1 &= ~( 1 << 4 );
      break;

    case 's':
    case 'S':
      /* Start */
      dwKeyPad1 &= ~( 1 << 3 );
      break;

    case 'a':
    case 'A':
      /* Select */
      dwKeyPad1 &= ~( 1 << 2 );
      break;

    case 'z':
    case 'Z':
      /* 'A' */
      dwKeyPad1 &= ~( 1 << 1 );
      break;

    case 'x':
    case 'X':
      /* 'B' */
      dwKeyPad1 &= ~( 1 << 0 );
      break;

#if 0
    case 'q':
    case 'Q':
      /* Terminate emulation thread */
      dwKeySystem &= ~( PAD_SYS_QUIT );
      break;
#endif

    defalut:
      break;
  }
}
#endif

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
    tid = rt_thread_create("nes_emu",
                           emulation_thread, RT_NULL,
                           4096, RT_THREAD_PRIORITY_LOW, RT_THREAD_TICK_DEFAULT);

    if (tid != RT_NULL)
        rt_thread_startup(tid);
  }
}

#if 0
/* Wrapper function for GTK file selection */
gint start_application_aux( GtkObject *gfs )
{
  /* Call actual function */
  start_application( gtk_file_selection_get_filename( GTK_FILE_SELECTION( gfs ) ) );

  /* Destroy a file selection widget */
  gtk_widget_destroy( GTK_WIDGET( gfs ) );
}
#endif

#if 0
/*===================================================================*/
/*                                                                   */
/*     close_application() : When invoked via signal delete_event    */
/*                                                                   */
/*===================================================================*/
gint close_application( GtkWidget *widget, GdkEvent *event, gpointer data )
{
  if ( bThread == TRUE )
  {
    /* Terminate emulation thread */
    bThread = FALSE;
    dwKeySystem |= PAD_SYS_QUIT;

    /* Leave Critical Section */
    gdk_threads_leave();

    /* Waiting for Termination */
    pthread_join( emulation_tid, NULL );

    /* Enter Critical Section */
    gdk_threads_enter();

    /* Save SRAM*/
    SaveSRAM();

    /* Release GC*/
    gdk_gc_destroy(gc);
  }

  /* Terminates the application */
  gtk_main_quit();

  return( FALSE );
}
#endif

/*===================================================================*/
/*                                                                   */
/*     reset_application() : Reset NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void reset_application( void )
{
#if 0
  /* Do nothing if emulation thread does not exists */
  if ( bThread == TRUE )
  {
    /* Terminate emulation thread */
    bThread = FALSE;
    dwKeySystem |= PAD_SYS_QUIT;

    /* Leave Critical Section */
    gdk_threads_leave();

    /* Waiting for Termination */
    pthread_join( emulation_tid, NULL );

    /* Enter Critical Section */
    gdk_threads_enter();

    /* Save SRAM File */
    SaveSRAM();

    /* Load cassette */
    if ( InfoNES_Load ( szRomName ) == 0 )
    {
      /* Load SRAM */
      LoadSRAM();

      /* Create Emulation Thread */
      bThread = TRUE;
      pthread_create( &emulation_tid, NULL, emulation_thread, NULL );
    }
  }
#endif
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
#if 0
  /* If terminated */
  if ( bThread == FALSE )
  {
    return -1;
  }
#endif

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
      /* 555 to RGB565 */
      canvas_buffer[ ( y << 8 ) + x ] = (wColor & 0x7c00) << 1
      | (wColor & 0x03e0) << 1 | (wColor & 0x0200) >> 4
      | (wColor & 0x001f) ;
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
  sound_fd = 0;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate )
{
  int tmp;
  int result;
  int sound_rate;
  int sound_frag;

  waveptr = 0;
  wavflag = 0;
#if 0
  /* Open sound device */
  sound_fd = open( SOUND_DEVICE, O_WRONLY );
  if ( sound_fd < 0 )
  {
    InfoNES_MessageBox("opening "SOUND_DEVICE"...failed");
    sound_fd = 0;
    return 0;
  } else {

  }

  /* Setting unsigned 8-bit format */
  tmp = AFMT_U8;
  result = ioctl(sound_fd, SNDCTL_DSP_SETFMT, &tmp);
  if ( result < 0 )
  {
    InfoNES_MessageBox("setting unsigned 8-bit format...failed");
    close(sound_fd);
    sound_fd = 0;
    return 0;
  } else {

  }

  /* Setting mono mode */
  tmp = 0;
  result = ioctl(sound_fd, SNDCTL_DSP_STEREO, &tmp);
  if (result < 0)
  {
    InfoNES_MessageBox("setting mono mode...failed");
    close(sound_fd);
    sound_fd = 0;
    return 0;
  } else {

  }

  sound_rate = sample_rate;
  result = ioctl(sound_fd, SNDCTL_DSP_SPEED, &sound_rate);
  if ( result < 0 )
  {
    InfoNES_MessageBox("setting sound rate...failed");
    close(sound_fd);
    sound_fd = 0;
    return 0;
  } else {

  }

  /* high word of sound_frag is number of frags, low word is frag size */
  sound_frag = 0x00080008;
  result = ioctl(sound_fd, SNDCTL_DSP_SETFRAGMENT, &sound_frag);
  if (result < 0)
  {
    InfoNES_MessageBox("setting soundfrags...failed");
    close(sound_fd);
    sound_fd = 0;
    return 0;
  } else {

  }
#endif
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
#if 0
  if ( sound_fd )
  {
    close(sound_fd);
  }
#endif
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 )
{
  int i;

  if ( sound_fd )
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
#if 0
    if ( wavflag )
    {
      if ( write( sound_fd, &final_wave[(wavflag - 1) << 10], 1024) < 1024 )
      {
	InfoNES_MessageBox( "wrote less than 1024 bytes\n" );
      }
      wavflag = 0;
    }
#endif
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
void InfoNES_MessageBox( char *pszMsg, ... )
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
