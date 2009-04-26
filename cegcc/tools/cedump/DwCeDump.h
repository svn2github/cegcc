#ifndef	__DwCeDump_H__
#define	__DwCeDump_H__

typedef unsigned int RVA;
typedef	unsigned int ULONG32;
typedef unsigned long ULONG64;
typedef wchar_t WCHAR;
typedef unsigned short USHORT;
typedef unsigned long DWORD;

/*
 * This structure identifies the type, version, and format
 * of a minidump file.
 *
 * Signature must be one of :
 *	CEDUMP_SIGNATURE_CONTEXT ('XDEC')
 *	CEDUMP_SIGNATURE_SYSTEM   ('SDEC')
 *	CEDUMP_SIGNATURE_COMPLETE ('CDEC')
 */
typedef struct _MINIDUMP_HEADER {
	ULONG32 Signature;
	ULONG32 Version;
	ULONG32 NumberOfStreams;
	RVA StreamDirectoryRva;		/* relative virtual address of
					   an array of MINIDUMP_DIRECTORY */
	ULONG32 CheckSum;
	union
	{
		ULONG32 Reserved;
		ULONG32 TimeDateStamp;
	};
	ULONG64 Flags;
} MINIDUMP_HEADER, *PMINIDUMP_HEADER;

#define	CEDUMP_SIGNATURE_CONTEXT	"XDEC"
#define	CEDUMP_SIGNATURE_SYSTEM		"SDEC"
#define	CEDUMP_SIGNATURE_COMPLETE	"CDEC"

/*
 * This structure contains information about the location of memory ranges
 * specified in _MINIDUMP_MEMORY_DESCRIPTOR.
 */
typedef struct _MINIDUMP_LOCATION_DESCRIPTOR {
	ULONG32 DataSize;
	RVA Rva;
} MINIDUMP_LOCATION_DESCRIPTOR;

/*
 * This enumeration lists the valid kinds of Windows CE stream types.
 */
typedef enum _MINIDUMP_STREAM_TYPE {
	UnusedStream                = 0,
// 1 to 0x7FFF - Reserved for Desktop Windows
// 0x8000 to 0xFFFE - Windows CE stream types
	ceStreamNull                = 0x8000,
	ceStreamSystemInfo          = 0x8001,
	ceStreamException           = 0x8002,
	ceStreamModuleList          = 0x8003,
	ceStreamProcessList         = 0x8004,
	ceStreamThreadList          = 0x8005, 
	ceStreamThreadContextList   = 0x8006,
	ceStreamThreadCallStackList = 0x8007,
	ceStreamMemoryVirtualList   = 0x8008,
	ceStreamMemoryPhysicalList  = 0x8009,
	ceStreamBucketParameters    = 0x800A,     

	LastReservedStream          = 0xffff

} MINIDUMP_STREAM_TYPE;

/*
 * This structure identifies and locates each stream type
 * found in an error report minidump.
 */
typedef struct _MINIDUMP_DIRECTORY {
	MINIDUMP_STREAM_TYPE StreamType;
	MINIDUMP_LOCATION_DESCRIPTOR Location;
} MINIDUMP_DIRECTORY, *PMINIDUMP_DIRECTORY;

/*
 * This structure contains information about a memory range
 * listed in _CEDUMP_MEMORY_LIST.
 */
typedef struct _MINIDUMP_MEMORY_DESCRIPTOR {
	ULONG64 StartOfMemoryRange;
	MINIDUMP_LOCATION_DESCRIPTOR Memory;
} MINIDUMP_MEMORY_DESCRIPTOR, *PMINIDUMP_MEMORY_DESCRIPTOR;

/*
 * This structure describes the format of a string pointed to by an RVA pointer.
 */
typedef struct _MINIDUMP_STRING {
	ULONG32 Length;
	WCHAR Buffer [0];
} MINIDUMP_STRING, *PMINIDUMP_STRING;

/*
 * This structure defines the bucketing parameters used when
 * uploading an error report. 
 * This structure is used to define the ceStreamBucketParameters stream.
 */
typedef struct _CEDUMP_BUCKET_PARAMETERS {
	USHORT SizeOfHeader;
	USHORT __unusedAlignment;
	RVA EventType;
	ULONG32 fDebug;		/* Type of app build, 1 = debug, 0 = release */
	RVA AppName;
	ULONG32 AppStamp;
	ULONG32 AppVerMS;
	ULONG32 AppVerLS;
	RVA ModName; 
	ULONG32 ModStamp;
	ULONG32 ModVerMS;
	ULONG32 ModVerLS;
	ULONG32 Offset;
	RVA OwnerName;
	ULONG32 OwnerStamp;
	ULONG32 OwnerVerMS;
	ULONG32 OwnerVerLS;
} CEDUMP_BUCKET_PARAMETERS, *PCEDUMP_BUCKET_PARAMETERS;

/*
 * This structure contains a list of all the modules, processes,
 * threads, or thread contexts that were active when the dump file
 * was generated.
 */
typedef struct _CEDUMP_ELEMENT_LIST {
	USHORT SizeOfHeader;
	USHORT SizeOfFieldInfo; 
	ULONG32 NumberOfFieldInfo; 
	ULONG32 NumberOfElements; 
	RVA Elements; 
} CEDUMP_ELEMENT_LIST, *PCEDUMP_ELEMENT_LIST;

/*
 * This structure describes fields associated with modules, processes,
 * threads, and context records listed in _CEDUMP_ELEMENT_LIST.
 */
typedef struct _CEDUMP_FIELD_INFO {
	ULONG32 FieldId;
	ULONG32 FieldSize;
	RVA FieldLabel;
	RVA FieldFormat;
} CEDUMP_FIELD_INFO, *PCEDUMP_FIELD_INFO;

/*
 * This structure contains system-wide information about the device
 * and operating system with which the dump file was generated.
 */
typedef struct _CEDUMP_SYSTEM_INFO {
	USHORT SizeOfHeader;
	USHORT ProcessorArchitecture;
	ULONG32 NumberOfProcessors;
	ULONG32 ProcessorType;
	USHORT ProcessorLevel;
	USHORT ProcessorRevision;
	ULONG32 ProcessorFamily;
	ULONG32 MajorVersion;
	ULONG32 MinorVersion;
	ULONG32 BuildNumber;
	ULONG32 PlatformId;
	ULONG32 LCID; 
	RVA OEMStringRva;
	ULONG32 SupportFlags;
	USHORT Machine;
	USHORT __unusedAlignment;
	RVA PlatformTypeRva;
	DWORD Platforms;
	RVA PlatformVersion;
	DWORD InstructionSet;
	DWORD __unusedAlignment2;
} CEDUMP_SYSTEM_INFO, *PCEDUMP_SYSTEM_INFO;

/*
 * This structure lists the memory dumps associated with the error report.
 */
typedef struct _CEDUMP_MEMORY_LIST {
	USHORT SizeOfHeader;
	USHORT SizeOfEntry;
	ULONG32 NumberOfEntries; 
} CEDUMP_MEMORY_LIST, *PCEDUMP_MEMORY_LIST;

/*
 * This structure contains the exception record for the exception
 * that caused the dump file to be generated.
 */
#define	EXCEPTION_MAXIMUM_PARAMETERS	8	/* ?? */
typedef struct _CEDUMP_EXCEPTION {
	ULONG32 ExceptionCode;
	ULONG32 ExceptionFlags;
	ULONG32 ExceptionRecord;
	ULONG32 ExceptionAddress;
	ULONG32 NumberParameters;
	ULONG32 ExceptionInformation [ EXCEPTION_MAXIMUM_PARAMETERS ];
} CEDUMP_EXCEPTION, *PCEDUMP_EXCEPTION;

/*
 * This structure contains information about the exception that
 * caused the dump file to be generated.
 */
typedef struct _CEDUMP_EXCEPTION_STREAM {
	USHORT SizeOfHeader;
	USHORT SizeOfException;
	USHORT SizeOfThreadContext; 
	USHORT Flags; 
	ULONG32 CurrentProcessId;
	ULONG32 ThreadId;
	ULONG32 OwnerProcessId;
	ULONG32 CaptureAPICurrentProcessId;
	ULONG32 CaptureAPIThreadId;
	ULONG32 CaptureAPIOwnerProcessId;
} CEDUMP_EXCEPTION_STREAM, *PCEDUMP_EXCEPTION_STREAM;

/*
 * The thread call stack is a header for stack frames.
 */
typedef struct _CEDUMP_THREAD_CALL_STACK {
	ULONG32 ProcessId;
	ULONG32 ThreadId;
	USHORT SizeOfFrame; 
	USHORT NumberOfFrames;
	RVA StackFramess;
} CEDUMP_THREAD_CALL_STACK, *PCEDUMP_THREAD_CALL_STACK;

/*
 * This structure contains information about the thread call stack
 * frames listed in _CEDUMP_THREAD_CALL_STACK.
 */
typedef struct _CEDUMP_THREAD_CALL_STACK_FRAME {
	ULONG32 ReturnAddr;
	ULONG32 FramePtr;
	ULONG32 ProcessId;
	ULONG32 __unusedAlignment;
	ULONG32 Params[4];
} CEDUMP_THREAD_CALL_STACK_FRAME, *PCEDUMP_THREAD_CALL_STACK_FRAME;

/*
 * The structure contains a list of the thread call stacks
 * associated with the dump file.
 */
typedef struct _CEDUMP_THREAD_CALL_STACK_LIST {
	USHORT SizeOfHeader;
	USHORT SizeOfEntry;
	ULONG32 NumberOfEntries;
} CEDUMP_THREAD_CALL_STACK_LIST, *PCEDUMP_THREAD_CALL_STACK_LIST;

#endif
