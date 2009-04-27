/*
 * cedump
 *
 * Portable application that inspects a CE dump file and reports about
 * its contents. This is type type of information that CE offers to send
 * to Microsoft free of charge to you.
 *
 * LICENSE:
 *
 * Copyright (c) 2009, Danny Backx.
 *
 * This file is part of CeGCC.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * Dump File Format
 *
 * http://msdn.microsoft.com/en-us/library/ms939593.aspx
 * 
 * The Error Reporting dump format uses Relative Virtual Addresses (RVA)
 * to describe the location of a data member within a file.
 * An RVA is an offset from the beginning of a file.
 * 
 * Because a minidump for a target device must be readable on another
 * device, all structures in the dump file are the same size, regardless
 * of the architecture of the faulting device.
 * 
 * The dump file format specifies a set of directories that point to the data.
 *
 * Each directory specifies the following:
 * 	* Data type
 *	*  Data size
 *	*  RVA to the location of the data in the dump file
 *
 * Each file permits only one directory of a specific type.
 * The basic dump file format is a single MINIDUMP_HEADER followed
 * by n MINIDUMP_DIRECTORY entries followed by n data entries.
 *
 * The MINIDUMP_HEADER specifies how many MINIDUMP_DIRECTORY entries follow.
 * Each MINIDUMP_DIRECTORY entry points to data in the dump data section.
 *
 * The following diagram shows the relationship of the data structures
 * used in dump files.
 *
 * MINIDUMP_HEADER
 * Signature
 * Version
 * NumberOfStreams
 * StreamDirectoryRva
 * Directory [ 0 ]
 *   StreamType = ceStreamSystemInfo
 *   DataSize
 *   DataRva
 *          
 * Directory [ 1 ]
 *   StreamType = ceStreamException
 *   DataSize
 *   DataRva
 *
 * Directory [NumberOfStreams – 1]
 *   StreamType = ceStreamThreadContextList
 *   DataSize
 *   DataRva
 *           
 * Data [ 0 ] (_CEDUMP_SYSTEM_INFO)
 *   SizeOfHeader
 *   ProcessorArchitecture
 *   NumberOfProcessors
 *   
 * Data [ 1 ] (_CEDUMP_EXCEPTION_STREAM)
 *   SizeOfHeader
 *   SizeOfException
 *   SizeOfThreadContext
 *
 * Data [NumberOfStreams – 1](_CEDUMP_ELEMENT_LIST)
 *   SizeOfHeader
 *   SizeOfFieldInfo
 *   NumberOfFieldInfo
 *   NumberOfElements
 *   Elements
 *   FieldInfo [ 0 ] (_CEDUMP_FIELD_INFO)
 *      FieldId
 *      FieldSize
 *      FieldLabel
 *      FieldFormat
 */
#ifndef	__DwCeDump_H__
#define	__DwCeDump_H__

typedef unsigned int RVA;	/* Offset from beginning of file */
typedef	unsigned int ULONG32;
typedef unsigned long ULONG64;
typedef wchar_t WCHAR;		/* beware : differs between host and target */
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
	USHORT SizeOfHeader;	/* Size of this structure */
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
	USHORT SizeOfHeader;	/* Size of this structure */
	USHORT ProcessorArchitecture;	/* Same as wProcessorArchitecture from SYSTEM_INFO structure */
	ULONG32 NumberOfProcessors;	/* Same as dwNumberOfProcessors from SYSTEM_INFO structure */
	ULONG32 ProcessorType;		/* Same as wProcessorType from SYSTEM_INFO */
	USHORT ProcessorLevel;		/* Same as wProcessorLevel from SYSTEM_INFO */
	USHORT ProcessorRevision;	/* .. */
	ULONG32 ProcessorFamily;	/* PROCESSOR_FAMILY_ ARM/SH4/MIPS4/MIPS/X86 */
	ULONG32 MajorVersion;	/* Same as dwMajorVersion of OSVERSIONINFO */
	ULONG32 MinorVersion;	/* Same as dwMinorVersion of OSVERSIONINFO */
	ULONG32 BuildNumber;	/* Same as dwBuildNumber of OSVERSIONINFO */
	ULONG32 PlatformId;	/* Same as dwPlatformId of OSVERSIONINFO */
	ULONG32 LCID; 		/* Locale identifier */
	RVA OEMStringRva;	/* RVA of the OEM string in the dump file */
	ULONG32 SupportFlags;	/* Reserved for future use */
	USHORT Machine;		/* IMAGE_FILE_MACHINE_ I386/IA64/AMD64/ALPHA/POWERPC */
	USHORT __unusedAlignment;
	RVA PlatformTypeRva;	/* RVA of platform type string, may have embedded NULL characters */
	DWORD Platforms;	/* Numer of structures pointed to by PlatformVersion */
	RVA PlatformVersion;	/* Points to an array of PLATFORMVERSION */
	DWORD InstructionSet;	/* Result of QueryInstructionSet */
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
#define	EXCEPTION_MAXIMUM_PARAMETERS	15	/* Max 15 parameters of 4 bytes each */
typedef struct _CEDUMP_EXCEPTION {
	ULONG32 ExceptionCode;
	ULONG32 ExceptionFlags;
	ULONG32 ExceptionRecord;
	ULONG32 ExceptionAddress;
	ULONG32 NumberParameters;
	ULONG32 ExceptionInformation [ 0 ];	/* placeholder */
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
		/* Bitfield :
		 * bit 0 this is a first chance exception dump
		 * bit 1 CaptureDumpFileOnDevice called
		 * bit 2 ReportFault called
		 * bit 3 exception thread id matches dump thread
		 */
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
	RVA StackFrames;
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
