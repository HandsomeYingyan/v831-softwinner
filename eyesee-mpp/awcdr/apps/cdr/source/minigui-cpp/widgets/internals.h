/*****************************************************************************
 Copyright (C), 2015, AllwinnerTech. Co., Ltd.
 File name: internals.h
 Author: yangy@allwinnertech.com
 Version: v1.0
 Date: 2015-11-24
 Description:

 History:
*****************************************************************************/


#ifndef GUI_INTERNALS_H
    #define GUI_INTERNALS_H

/******************* Internal data *******************************************/

/* Internal window message. */
#if (defined(_MG_ENABLE_SCREENSAVER) || defined(_MG_ENABLE_WATERMARK))
        && defined(_MGRM_THREADS)
#define MSG_CANCELSCREENSAVER   0x0201

#if defined(__GNUC__) && (__GNUC__ >= 4)
__attribute__ ((visibility ("hidden"))) void screensaver_hide(void);
#else
void screensaver_hide(void);
#endif
#endif

/* Internal use extended style. */
#define WS_EX_MODALDISABLED     0x10000000L

/**
 * \def WS_EX_CTRLASMAINWIN
 * \brief The control can be displayed out of the main window
 *        which contains the control.
 */
#define WS_EX_CTRLASMAINWIN     0x20000000L

#define DEF_NR_TIMERS       32

#if defined (__NOUNIX__) || defined (__uClinux__)
  #define DEF_MSGQUEUE_LEN    16
  #define SIZE_CLIPRECTHEAP   16
  #define SIZE_INVRECTHEAP    32
  #define SIZE_QMSG_HEAP      16
#else
 #ifndef _MGRM_THREADS
  #define DEF_MSGQUEUE_LEN    16
  #define SIZE_CLIPRECTHEAP   16
  #define SIZE_INVRECTHEAP    32
  #define SIZE_QMSG_HEAP      16
 #else
  #define DEF_MSGQUEUE_LEN    16     /* default message queue length */
  #define SIZE_CLIPRECTHEAP   128
  #define SIZE_INVRECTHEAP    128
  #define SIZE_QMSG_HEAP      8
 #endif
#endif

/* IME info define.*/
#define IME_SET_STATUS        1
#define IME_GET_STATUS        2
#define IME_SET_POS           3
#define IME_GET_POS           4

/******************* Internal data of fix string module **********************/
#if defined (__NOUNIX__) || defined (__uClinux__)
  #define MAX_LEN_FIXSTR      64
  #define NR_HEAP             5
  #define LEN_BITMAP          (1+2+4+8+16)
#else
 #ifdef _MGRM_THREADS
  #define MAX_LEN_FIXSTR      2048
  #define NR_HEAP             10
  #define LEN_BITMAP          (1+2+4+8+16+32+64+128+256+512)
 #else
  #define MAX_LEN_FIXSTR      64
  #define NR_HEAP             5
  #define LEN_BITMAP          (1+2+4+8+16)
 #endif
#endif

/******************** Handle type and child type. ***************************/
#define TYPE_HWND           0x01
    #define TYPE_MAINWIN    0x11
    #define TYPE_CONTROL    0x12
    #define TYPE_ROOTWIN    0x13
#define TYPE_HMENU          0x02
    #define TYPE_MENUBAR    0x21
    #define TYPE_PPPMENU    0x22
    #define TYPE_NMLMENU    0x23
#define TYPE_HACCEL         0x03
#define TYPE_HCURSOR        0x05
#define TYPE_HICON          0x07
#define TYPE_HDC            0x08
    #define TYPE_SCRDC      0x81
    #define TYPE_GENDC      0x82
    #define TYPE_MEMDC      0x83

#define TYPE_WINTODEL       0xF1
#define TYPE_UNDEFINED      0xFF

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

struct _MAINWIN;
typedef struct _MAINWIN* PMAINWIN;

#include "zorder.h"

typedef struct _SCROLLWINDOWINFO
{
    int iOffx;
    int iOffy;
    const RECT* rc1;
    const RECT* rc2;
}SCROLLWINDOWINFO;
typedef SCROLLWINDOWINFO* PSCROLLWINDOWINFO;

typedef struct _CARETINFO {
    int     x;              // position of caret
    int     y;
    void*   pNormal;        // normal bitmap.
    void*   pXored;         // bit-Xored bitmap.

    PBITMAP pBitmap;        // user defined caret bitmap.

    int     nWidth;         // original size of caret
    int     nHeight;
    int     nBytesNr;       // number of bitmap bytes.

    BITMAP  caret_bmp;      // bitmap of caret.

    BOOL    fBlink;         // does blink?
    BOOL    fShow;          // show or hide currently.

    HWND    hOwner;         // the window owns the caret.
    UINT    uTime;          // the blink time.
}CARETINFO;
typedef CARETINFO* PCARETINFO;

/* ime status info */
typedef struct _IME_STATUS_INFO {
    HWND hwnd;
    BYTE bEnabled;
    BYTE bAutoTrack;
    BYTE reserved[2];
    IME_TARGET_INFO TargetInfo;
} IME_STATUS_INFO;

typedef struct _QMSG
{
    MSG                 Msg;
    struct _QMSG*       next;
}QMSG;
typedef QMSG* PQMSG;

typedef struct _MSGQUEUE MSGQUEUE;
typedef MSGQUEUE* PMSGQUEUE;

#ifdef _MGRM_THREADS
typedef struct _SYNCMSG
{
    MSG              Msg;
    int              retval;
    sem_t*           sem_handle;
    struct _SYNCMSG* pNext;
}SYNCMSG;
typedef SYNCMSG* PSYNCMSG;
#else
typedef BOOL (* IDLEHANDLER) (PMSGQUEUE msg_que);
#endif

// the MSGQUEUE struct is a internal struct.
// using semaphores to implement message queue.
struct _MSGQUEUE
{
    DWORD dwState;              // message queue states

#ifdef _MGRM_THREADS
    pthread_mutex_t lock;       // lock
    sem_t wait;                 // the semaphore for wait message
    sem_t sync_msg;             // the semaphore for sync message
#endif

    PQMSG  pFirstNotifyMsg;     // head of the notify message queue
    PQMSG  pLastNotifyMsg;      // tail of the notify message queue

#ifdef _MGRM_THREADS
    PSYNCMSG pFirstSyncMsg;     // head of the sync message queue
    PSYNCMSG pLastSyncMsg;      // tail of the sync message queue
#else
    IDLEHANDLER OnIdle;         // Idle handler
#endif

#ifdef _MGRM_THREADS
    PMAINWIN pRootMainWin;      // The root main window of this message queue.
#endif

    MSG* msg;                   /* post message buffer */
    int len;                    /* buffer len */
    int readpos, writepos;      /* positions for reading and writing */

    int FirstTimerSlot;         /* the first timer slot to be checked */
    DWORD TimerMask;            /* timer slots mask */

    int loop_depth;             /* message loop depth, for dialog boxes. */
};

BOOL mg_InitFreeQMSGList (void);
void mg_DestroyFreeQMSGList (void);
BOOL mg_InitMsgQueue (PMSGQUEUE pMsgQueue, int iBufferLen);
void mg_DestroyMsgQueue (PMSGQUEUE pMsgQueue);
BOOL kernel_QueueMessage (PMSGQUEUE msg_que, PMSG msg);

extern PMSGQUEUE __mg_dsk_msg_queue;

/* Running */
#define _MG_QUITING_STAGE_RUNNING 1
/* Try to quit main thread,
        and for Threads version quit other MainWindow Thread */
#define _MG_QUITING_STAGE_START   (-10)
/* Force to quit main thread, and other MainWindow Thread */
#define _MG_QUITING_STAGE_FORCE   (-15)
/* Quit Desktop thread (Threads version only) */
#define _MG_QUITING_STAGE_DESKTOP (-20)
/* Quit EventLoop (Threads version only) */
#define _MG_QUITING_STAGE_EVENT   (-30)
/* Quit TimerEntry (Threads version only) */
#define _MG_QUITING_STAGE_TIMER   (-40)
extern int __mg_quiting_stage;

static inline BOOL QueueDeskMessage (PMSG msg)
{
    return kernel_QueueMessage (__mg_dsk_msg_queue, msg);
}

#ifndef _MGRM_THREADS
static inline BOOL InitDskMsgQueue (void)
{
    return mg_InitMsgQueue (__mg_dsk_msg_queue, 0);
}

static inline void DestroyDskMsgQueue (void)
{
    mg_DestroyMsgQueue (__mg_dsk_msg_queue);
}

static inline void SetDskIdleHandler (IDLEHANDLER idle_handler)
{
    __mg_dsk_msg_queue->OnIdle = idle_handler;
}

#endif

#ifdef _MGRM_THREADS

  #define MG_MUTEX_INIT(lock)      pthread_mutex_init(lock, NULL)
  #define MG_MUTEX_DESTROY(lock)   pthread_mutex_destroy(lock)
  #define MG_MUTEX_LOCK(lock)      pthread_mutex_lock(lock)
  #define MG_MUTEX_UNLOCK(lock)    pthread_mutex_unlock(lock)

  #define POST_MSGQ(pMsgQueue) \
  do { \
    int sem_value; \
    /* Signal that the msg queue contains one more element for reading */ \
    sem_getvalue (&(pMsgQueue)->wait, &sem_value); \
    if (sem_value <= 0) {\
        sem_post(&(pMsgQueue)->wait); \
    } \
  } while(0)

#else

  #define MG_MUTEX_INIT(lock)
  #define MG_MUTEX_DESTROY(lock)
  #define MG_MUTEX_LOCK(lock)
  #define MG_MUTEX_UNLOCK(lock)

  #define POST_MSGQ(pMsgQueue)

#endif

#define LOCK_MSGQ(pMsgQueue)     MG_MUTEX_LOCK(&(pMsgQueue)->lock)
#define UNLOCK_MSGQ(pMsgQueue)   MG_MUTEX_UNLOCK(&(pMsgQueue)->lock)

struct _wnd_element_data;

#define WF_ERASEBKGND    0x01 //flags to erase bkground or not

// this struct is an internal struct
typedef struct _MAINWIN
{
    /*
     * These fields are similiar with CONTROL struct.
     */
    unsigned char DataType;     // the data type.
    unsigned char WinType;      // the window type.
    unsigned short Flags;       // special runtime flags, such EraseBkGnd flags

    int left, top;      // the position and size of main window.
    int right, bottom;

    int cl, ct;         // the position and size of client area.
    int cr, cb;

    DWORD dwStyle;      // the styles of main window.
    DWORD dwExStyle;    // the extended styles of main window.

    int iBkColor;       // the background color.
    HMENU hMenu;        // handle of menu.
    HACCEL hAccel;      // handle of accelerator table.
    HCURSOR hCursor;    // handle of cursor.
    HICON hIcon;        // handle of icon.
    HMENU hSysMenu;     // handle of system menu.
    PLOGFONT pLogFont;  // pointer to logical font.

    char* spCaption;    // the caption of main window.
    int   id;           // the identifier of main window.

    LFSCROLLBARINFO vscroll;
                        // the vertical scroll bar information.
    LFSCROLLBARINFO hscroll;
                        // the horizital scroll bar information.

    /** the window renderer */
    WINDOW_ELEMENT_RENDERER* we_rdr;

    HDC   privCDC;      // the private client DC.
    INVRGN InvRgn;      // the invalid region of this main window.
    PGCRINFO pGCRInfo;  // pointer to global clip region info struct.

    // the Z order node.
    int idx_znode;

    PCARETINFO pCaretInfo;
                        // pointer to system caret info struct.

    DWORD dwAddData;    // the additional data.
    DWORD dwAddData2;   // the second addtional data.

    int (*MainWindowProc)(HWND, int, WPARAM, LPARAM);
                           // the address of main window procedure.

    struct _MAINWIN* pMainWin;
                        // the main window that contains this window.
                        // for main window, always be itself.

    HWND hParent;       // the parent of this window.
                        // for main window, always be HWND_DESKTOP.

    /*
     * Child windows.
     */
    HWND hFirstChild;    // the handle of first child window.
    HWND hActiveChild;  // the currently active child window.
    HWND hOldUnderPointer;  // the old child window under pointer.
    HWND hPrimitive;    // the premitive child of mouse event.

    NOTIFPROC NotifProc;    // the notification callback procedure.

    /*
     * window element data.
     */
    struct _wnd_element_data* wed;

    /*
     * Main Window hosting.
     * The following members are only implemented for main window.
     */
    struct _MAINWIN* pHosting;
                        // the hosting main window.
    struct _MAINWIN* pFirstHosted;
                        // the first hosted main window.
    struct _MAINWIN* pNextHosted;
                        // the next hosted main window.

    PMSGQUEUE pMessages;
                        // the message queue.

    GCRINFO GCRInfo;
                        // the global clip region info struct.
                        // put here to avoid invoking malloc function.

#ifdef _MGRM_THREADS
    pthread_t th;        // the thread which creates this main window.
#endif
    //the controls as main
    HWND hFirstChildAsMainWin;

    HDC   secondaryDC;                // the secondary window dc.
    ON_UPDATE_SECONDARYDC update_secdc; // the callback of secondary window dc
    RECT  update_rc;
} MAINWIN;

/************************* Initialization/Termination ************************/

void __mg_init_local_sys_text (void);

/* the zorder information of the server */
extern ZORDERINFO* __mg_zorder_info;

#ifndef _MGRM_THREADS

#ifdef _MGRM_STANDALONE

BOOL salone_IdleHandler4StandAlone (PMSGQUEUE msg_que);
BOOL salone_StandAloneStartup (void);
void salone_StandAloneCleanup (void);

#else

/*
 * Common for server and client.
 */

/* the client id is zero for the server */
extern int __mg_client_id;

BOOL kernel_IsOnlyMe (void);
void* kernel_LoadSharedResource (void);
void kernel_UnloadSharedResource (void);
BOOL server_IdleHandler4Server (PMSGQUEUE msg_que);
void server_ServerCleanup (void);

/* Only for client. */
void* kernel_AttachSharedResource (void);
void kernel_UnattachSharedResource (void);
BOOL client_IdleHandler4Client (PMSGQUEUE msg_que);
BOOL client_ClientStartup (void);
void client_ClientCleanup (void);

#endif

#endif

BOOL mg_InitScreenDC (void* surface);
void mg_TerminateScreenDC (void);

BOOL mg_InitGDI (void);
void mg_TerminateGDI (void);

BOOL mg_InitFixStr (void);
void mg_TerminateFixStr (void);

BOOL mg_InitMenu (void);
void mg_TerminateMenu (void);

BOOL mg_InitDesktop (void);
void mg_TerminateDesktop (void);

/* send MSG_IME_OPEN/CLOSE message to ime window */
void gui_open_ime_window(PMAINWIN pWin, BOOL open_close, HWND rev_hwnd);

/* return main window contains hwnd. */
PMAINWIN gui_GetMainWindowPtrOfControl (HWND hwnd);

/* check whether hwnd is main window and return pointer to main window hwnd. */
PMAINWIN gui_CheckAndGetMainWindowPtr (HWND hwnd);

PMAINWIN gui_GetMainWindowPtrUnderPoint (int x, int y);

/* return message queue of window. */
PMSGQUEUE kernel_GetMsgQueue (HWND hwnd);

/* return the next window need to repaint. */
HWND kernel_CheckInvalidRegion (PMAINWIN pWin);

/* return global clipping region of window. */
PGCRINFO kernel_GetGCRgnInfo (HWND hwnd);

/* internal variables */
typedef struct _TRACKMENUINFO* PTRACKMENUINFO;

extern unsigned int __mg_timer_counter;

extern HWND __mg_capture_wnd;
extern HWND __mg_ime_wnd;
#ifndef _MGRM_PROCESSES
extern PMAINWIN __mg_active_mainwnd;
extern PTRACKMENUINFO __mg_ptmi;
#endif
extern PMAINWIN __mg_dsk_win;
extern HWND __mg_hwnd_desktop;

extern int DesktopWinProc (HWND hwnd,
                int message, WPARAM wparam, LPARAM lparam);
extern int SendSyncMessage (HWND hwnd, int msg, WPARAM wparam,
                LPARAM lparam);


#ifndef _MGRM_THREADS
    extern unsigned int __mg_csrimgsize;
    extern unsigned int __mg_csrimgpitch;
#else
    extern pthread_mutex_t __mg_gdilock, __mg_mouselock;
    extern pthread_t __mg_desktop, __mg_parsor, __mg_timer;
#endif


/* hwnd is a window, including HWND_DESKTOP */
#define MG_IS_WINDOW(hwnd)              \
            (hwnd &&                    \
             hwnd != HWND_INVALID &&    \
             ((PMAINWIN)hwnd)->DataType == TYPE_HWND)

/* hwnd is a normal window, not including HWND_DESKTOP */
#define MG_IS_NORMAL_WINDOW(hwnd)       \
        (hwnd != HWND_DESKTOP && MG_IS_WINDOW(hwnd))

/* hwnd is a main window */
#define MG_IS_MAIN_WINDOW(hwnd)         \
        (MG_IS_WINDOW(hwnd) && ((PMAINWIN)hwnd)->WinType == TYPE_MAINWIN)

#define MG_IS_NORMAL_MAIN_WINDOW(hwnd)  \
        (hwnd != HWND_DESKTOP && MG_IS_MAIN_WINDOW(hwnd))

#define MG_IS_DESTROYED_WINDOW(hwnd)    \
        (hwnd &&                        \
         (hwnd != HWND_INVALID) &&      \
        (((PMAINWIN)hwnd)->DataType == TYPE_WINTODEL))

#define MG_GET_WINDOW_PTR(hwnd)   ((PMAINWIN)hwnd)
#define MG_GET_CONTROL_PTR(hwnd)  ((PCONTROL)hwnd)

#define MG_CHECK_RET(condition, ret) \
            if (!(condition)) return ret

#define MG_CHECK(condition) \
            if (!(condition)) return

/* get main window pointer of a window, including desktop window */
static inline PMAINWIN getMainWindowPtr (HWND hwnd)
{
    PMAINWIN pWin;

    MG_CHECK_RET (MG_IS_WINDOW(hwnd), NULL);
    pWin = MG_GET_WINDOW_PTR (hwnd);

    return pWin->pMainWin;
}

/* -------------------------------------------------------------------------- */
#ifdef _MGRM_THREADS

/* Be careful: does not check validity of hwnd */
static inline BOOL BE_THIS_THREAD (HWND hwnd)
{
    PMAINWIN pMainWin = getMainWindowPtr(hwnd);
#ifdef WIN32
    if (pMainWin && pMainWin->th.p == pthread_self().p)
#else
    if (pMainWin && pMainWin->th == pthread_self())
#endif
        return 1;

    return FALSE;
}

MSGQUEUE* mg_InitMsgQueueThisThread (void);
void mg_FreeMsgQueueThisThread (void);

extern pthread_key_t __mg_threadinfo_key;
static inline MSGQUEUE* GetMsgQueueThisThread (void)
{
    MSGQUEUE* pMsgQueue;

    pMsgQueue = (MSGQUEUE*) pthread_getspecific (__mg_threadinfo_key);
#ifdef __VXWORKS__
    if (pMsgQueue == (void *)-1) {
    return NULL;
    }
#endif
    return pMsgQueue;
}

#endif

#ifndef _MGRM_THREADS
static inline void
SetDesktopTimerFlag (void)
{
    __mg_dsk_msg_queue->dwState |= QS_DESKTIMER;
}
#else
static inline void
AlertDesktopTimerEvent (void)
{
    if(__mg_dsk_msg_queue)
    {
        __mg_dsk_msg_queue->TimerMask = 1;
        POST_MSGQ(__mg_dsk_msg_queue);
    }
}
#endif

static inline void
SetMsgQueueTimerFlag (PMSGQUEUE pMsgQueue, int slot)
{
    pMsgQueue->TimerMask |= (0x01 << slot);
    POST_MSGQ (pMsgQueue);
}

static inline void
RemoveMsgQueueTimerFlag (PMSGQUEUE pMsgQueue, int slot)
{
    pMsgQueue->TimerMask &= ~(0x01 << slot);
}

BOOL mg_InitTimer (void);
BOOL mg_InstallIntervalTimer (void);
BOOL mg_UninstallIntervalTimer (void);

/*window element renderer manager interface*/
extern WINDOW_ELEMENT_RENDERER * __mg_def_renderer;

BOOL mg_InitLFManager (void);
void mg_TerminateLFManager (void);

char* gui_GetIconFile(const char* rdr_name, char* file, char* _szValue);
BOOL gui_LoadIconRes(HDC hdc, const char* rdr_name, char* file);

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_INTERNALS_H
