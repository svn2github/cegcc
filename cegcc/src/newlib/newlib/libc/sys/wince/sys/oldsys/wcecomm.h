#ifndef _WCESERIAL_H_
#define _WCESERIAL_H_

#include <sys/wcetypes.h>

/* Events */
#define EV_RXCHAR           0x0001  /* Any Character received */
#define EV_RXFLAG           0x0002  /* Received certain character */
#define EV_TXEMPTY          0x0004  /* Transmitt Queue Empty */
#define EV_CTS              0x0008  /* CTS changed state */
#define EV_DSR              0x0010  /* DSR changed state */
#define EV_RLSD             0x0020  /* RLSD changed state */
#define EV_BREAK            0x0040  /* BREAK received */
#define EV_ERR              0x0080  /* Line status error occurred */
#define EV_RING             0x0100  /* Ring signal detected */
#define EV_PERR             0x0200  /* Printer error occured */
#define EV_RX80FULL         0x0400  /* Receive buffer is 80 percent full */
#define EV_EVENT1           0x0800  /* Provider specific event 1 */
#define EV_EVENT2           0x1000  /* Provider specific event 2 */
#define EV_POWER			         0x2000  /* WINCE Power event */

#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define ONE5STOPBITS        1
#define TWOSTOPBITS         2

#define IGNORE              0

/* Baud rates at which communications devices operate */
#define CBR_110             110
#define CBR_300             300
#define CBR_600             600
#define CBR_1200            1200
#define CBR_2400            2400
#define CBR_4800            4800
#define CBR_9600            9600
#define CBR_14400           14400
#define CBR_19200           19200
#define CBR_38400           38400
#define CBR_56000           56000
#define CBR_57600           57600
#define CBR_115200          115200
#define CBR_128000          128000
#define CBR_256000          256000

/* PURGE function flags */
#define PURGE_TXABORT       0x0001  /* Kill pending/current writes to comm port */
#define PURGE_RXABORT       0x0002  /* Kill pending/current reads to comm port */
#define PURGE_TXCLEAR       0x0004  /* Kill the transmit queue if there */
#define PURGE_RXCLEAR       0x0008  /* Kill the typeahead buffer if there */

#define LPTx                0x80    /* Set if ID is for LPT device */


/* Data Bits */
#define DATABITS_5        ((WORD)0x0001)
#define DATABITS_6        ((WORD)0x0002)
#define DATABITS_7        ((WORD)0x0004)
#define DATABITS_8        ((WORD)0x0008)
#define DATABITS_16       ((WORD)0x0010)
#define DATABITS_16X      ((WORD)0x0020)

/* Stop and Parity bits */
#define STOPBITS_10       ((WORD)0x0001)
#define STOPBITS_15       ((WORD)0x0002)
#define STOPBITS_20       ((WORD)0x0004)
#define PARITY_NONE       ((WORD)0x0100)
#define PARITY_ODD        ((WORD)0x0200)
#define PARITY_EVEN       ((WORD)0x0400)
#define PARITY_MARK       ((WORD)0x0800)
#define PARITY_SPACE      ((WORD)0x1000)

typedef struct _COMMPROP {
  WORD wPacketLength;
  WORD wPacketVersion;
  DWORD dwServiceMask;
  DWORD dwReserved1;
  DWORD dwMaxTxQueue;
  DWORD dwMaxRxQueue;
  DWORD dwMaxBaud;
  DWORD dwProvSubType;
  DWORD dwProvCapabilities;
  DWORD dwSettableParams;
  DWORD dwSettableBaud;
  WORD wSettableData;
  WORD wSettableStopParity;
  DWORD dwCurrentTxQueue;
  DWORD dwCurrentRxQueue;
  DWORD dwProvSpec1;
  DWORD dwProvSpec2;
  WCHAR wcProvChar[1];
} COMMPROP,*LPCOMMPROP;


/* Set dwProvSpec1 to COMMPROP_INITIALIZED to indicate that wPacketLength
 * is valid before a call to GetCommProperties() */
#define COMMPROP_INITIALIZED ((DWORD)0xE73CF52E)

typedef struct _COMSTAT {
  DWORD fCtsHold : 1;
  DWORD fDsrHold : 1;
  DWORD fRlsdHold : 1;
  DWORD fXoffHold : 1;
  DWORD fXoffSent : 1;
  DWORD fEof : 1;
  DWORD fTxim : 1;
  DWORD fReserved : 25;
  DWORD cbInQue;
  DWORD cbOutQue;
} COMSTAT, *LPCOMSTAT;

/* DTR Control Flow Values */
#define DTR_CONTROL_DISABLE    0x00
#define DTR_CONTROL_ENABLE     0x01
#define DTR_CONTROL_HANDSHAKE  0x02

/* RTS Control Flow Values */
#define RTS_CONTROL_DISABLE    0x00
#define RTS_CONTROL_ENABLE     0x01
#define RTS_CONTROL_HANDSHAKE  0x02
#define RTS_CONTROL_TOGGLE     0x03

typedef struct _DCB {
  DWORD DCBlength;      /* sizeof(DCB)                     */
  DWORD BaudRate;       /* Baudrate at which running       */
  DWORD fBinary: 1;     /* Binary Mode (skip EOF check)    */
  DWORD fParity: 1;     /* Enable parity checking          */
  DWORD fOutxCtsFlow:1; /* CTS handshaking on output       */
  DWORD fOutxDsrFlow:1; /* DSR handshaking on output       */
  DWORD fDtrControl:2;  /* DTR Flow control                */
  DWORD fDsrSensitivity:1; /* DSR Sensitivity              */
  DWORD fTXContinueOnXoff: 1; /* Continue TX when Xoff sent */
  DWORD fOutX: 1;       /* Enable output X-ON/X-OFF        */
  DWORD fInX: 1;        /* Enable input X-ON/X-OFF         */
  DWORD fErrorChar: 1;  /* Enable Err Replacement          */
  DWORD fNull: 1;       /* Enable Null stripping           */
  DWORD fRtsControl:2;  /* Rts Flow control                */
  DWORD fAbortOnError:1; /* Abort all reads and writes on Error */
  DWORD fDummy2:17;     /* Reserved                        */
  WORD wReserved;       /* Not currently used              */
  WORD XonLim;          /* Transmit X-ON threshold         */
  WORD XoffLim;         /* Transmit X-OFF threshold        */
  BYTE ByteSize;        /* Number of bits/byte, 4-8        */
  BYTE Parity;          /* 0-4=None,Odd,Even,Mark,Space    */
  BYTE StopBits;        /* 0,1,2 = 1, 1.5, 2               */
  char XonChar;         /* Tx and Rx X-ON character        */
  char XoffChar;        /* Tx and Rx X-OFF character       */
  char ErrorChar;       /* Error replacement char          */
  char EofChar;         /* End of Input character          */
  char EvtChar;         /* Received Event character        */
  WORD wReserved1;      /* Fill for now.                   */
} DCB, *LPDCB;

typedef struct _COMMTIMEOUTS {
  DWORD ReadIntervalTimeout;          /* Maximum time between read chars. */
  DWORD ReadTotalTimeoutMultiplier;   /* Multiplier of characters.        */
  DWORD ReadTotalTimeoutConstant;     /* Constant in milliseconds.        */
  DWORD WriteTotalTimeoutMultiplier;  /* Multiplier of characters.        */
  DWORD WriteTotalTimeoutConstant;    /* Constant in milliseconds.        */
} COMMTIMEOUTS,*LPCOMMTIMEOUTS;

typedef struct _COMMCONFIG {
  DWORD dwSize;               /* Size of the entire struct */
  WORD wVersion;              /* version of the structure */
  WORD wReserved;             /* alignment */
  DCB dcb;                    /* device control block */
  DWORD dwProviderSubType;    /* ordinal value for identifying
                                 provider-defined data structure format*/
  DWORD dwProviderOffset;     /* Specifies the offset of provider specific
                                 data field in bytes from the start */
  DWORD dwProviderSize;       /* size of the provider-specific data field */
  WCHAR wcProviderData[1];    /* provider-specific data */
} COMMCONFIG,*LPCOMMCONFIG;

#ifdef __cplusplus
extern "C" {
#endif

BOOL ClearCommBreak(HANDLE);
BOOL ClearCommError(HANDLE, LPDWORD, LPCOMSTAT);
BOOL SetupComm(HANDLE, DWORD, DWORD);
BOOL EscapeCommFunction(HANDLE, DWORD);
BOOL GetCommMask(HANDLE, LPDWORD);
BOOL GetCommProperties(HANDLE, LPCOMMPROP);
BOOL GetCommState(HANDLE, LPDCB);
BOOL GetCommTimeouts(HANDLE, LPCOMMTIMEOUTS);
BOOL PurgeComm(HANDLE, DWORD);
BOOL SetCommBreak(HANDLE);
BOOL SetCommMask(HANDLE, DWORD);
BOOL SetCommState(HANDLE, LPDCB);
BOOL SetCommTimeouts(HANDLE, LPCOMMTIMEOUTS);
BOOL TransmitCommChar(HANDLE, char);
BOOL WaitCommEvent(HANDLE, LPDWORD, LPOVERLAPPED);

#ifdef __cplusplus
}
#endif

#endif
