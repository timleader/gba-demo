
using System;
using System.Runtime.InteropServices;

namespace GBA.Tests
{

    public static class mGBA
    {

        //---------------------------------------------------------------------
        internal const string NativeSymbolLocation = "mgba";

        //---------------------------------------------------------------------
        public static class Log
        {
            //---------------------------------------------------------------------
            public enum LogLevel
            {
                FATAL = 0x01,
                ERROR = 0x02,
                WARN = 0x04,
                INFO = 0x08,
                DEBUG = 0x10,
                STUB = 0x20,
                GAME_ERROR = 0x40,

                ALL = 0x7F
            };

            //---------------------------------------------------------------------
            [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
            public delegate void LogFunction(IntPtr lLoggerStructPtr, int lCategory, LogLevel lLevel, string lFormat, IntPtr lArgs);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void DebugVarFunction(UInt16 lFlags, string lName, UInt32 lAddress);

			//---------------------------------------------------------------------
			[StructLayout(LayoutKind.Sequential)]
            public struct Logger
            {
                public LogFunction LogFunc;
                public IntPtr Filter;

				public DebugVarFunction DebugVarsFunc;
			}

            //---------------------------------------------------------------------
            [DllImport(NativeSymbolLocation, EntryPoint = "mLogSetDefaultLogger", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetDefaultLogger(IntPtr lLoggerStructPtr);

			//---------------------------------------------------------------------
			[DllImport(NativeSymbolLocation, EntryPoint = "mLog", CallingConvention = CallingConvention.Cdecl)]
			public static extern void SendLog(int lCategory, LogLevel lLevel, string lFormat, __arglist);

		}

		//---------------------------------------------------------------------
		public static class Debug
		{
			//---------------------------------------------------------------------
			[DllImport(NativeSymbolLocation, EntryPoint = "mDebuggerSymbolLookup", CallingConvention = CallingConvention.Cdecl)]
			public static extern bool SymbolLookup(IntPtr lDebuggerSymbols, string lName, out int lValue, out int lSegment);

			//---------------------------------------------------------------------
			[StructLayout(LayoutKind.Sequential)]
			public struct DebuggerPlatform
			{
				public IntPtr Debugger;

				public IntPtr Init;				//	void (*init)(void* cpu, struct mDebuggerPlatform*);
				public IntPtr Deinit;			//	void (*deinit)(struct mDebuggerPlatform*);
				public IntPtr Entered;			//	void (*entered)(struct mDebuggerPlatform*, enum mDebuggerEntryReason, struct mDebuggerEntryInfo*);

				public IntPtr HasBreakpoints;   //	bool (* hasBreakpoints) (struct mDebuggerPlatform*);
				public IntPtr CheckBreakpoints; //	void (* checkBreakpoints) (struct mDebuggerPlatform*);
				public IntPtr ClearBreakpoint;  //	bool (* clearBreakpoint) (struct mDebuggerPlatform*, ssize_t id);

				public IntPtr SetBreakpoint;    //	ssize_t(*setBreakpoint)(struct mDebuggerPlatform*, const struct mBreakpoint*);
				public IntPtr ListBreakpoints;  //	void (* listBreakpoints) (struct mDebuggerPlatform*, struct mBreakpointList*);

				public IntPtr SetWatchpoint;    //	ssize_t(*setWatchpoint)(struct mDebuggerPlatform*, const struct mWatchpoint*);
				public IntPtr ListWatchpoints;  //	void (* listWatchpoints) (struct mDebuggerPlatform*, struct mWatchpointList*);

				public IntPtr Trace;            //	void (* trace) (struct mDebuggerPlatform*, char* out, size_t* length);

				public IntPtr GetRegister;      //	bool (* getRegister) (struct mDebuggerPlatform*, const char* name, int32_t* value);
				public IntPtr SetRegister;      //	bool (* setRegister) (struct mDebuggerPlatform*, const char* name, int32_t value);
				public IntPtr LookupIdentifier; //	bool (* LookupIdentifier) (struct mDebuggerPlatform*, const char* name, int32_t* value, int* segment);

				public IntPtr GetStackTraceMode; //	uint32_t(*getStackTraceMode)(struct mDebuggerPlatform*);
				public IntPtr SetStackTraceMode; //	void (* setStackTraceMode) (struct mDebuggerPlatform*, uint32_t mode);
				public IntPtr UpdateStackTrace; //	bool (* updateStackTrace) (struct mDebuggerPlatform* d);
			}

        }

        //---------------------------------------------------------------------
        public static class VirtualFileSystem
		{
			//---------------------------------------------------------------------
			[StructLayout(LayoutKind.Sequential)]
			public struct File
            {

				public IntPtr Close;    //	bool (*close)(struct VFile* vf);
				public IntPtr Seek;     //	off_t (*seek)(struct VFile* vf, off_t offset, int whence);
				public IntPtr Read;     //	ssize_t (*read)(struct VFile* vf, void* buffer, size_t size);
				public IntPtr Readline; //	ssize_t (*readline)(struct VFile* vf, char* buffer, size_t size);
				public IntPtr Write;    //	ssize_t (*write)(struct VFile* vf, const void* buffer, size_t size);
				public IntPtr Map;      //	void* (*map)(struct VFile* vf, size_t size, int flags);
				public IntPtr Unmap;    //	void (*unmap)(struct VFile* vf, void* memory, size_t size);
				public IntPtr Truncate; //	void (*truncate)(struct VFile* vf, size_t size);
				public IntPtr Size;     //	ssize_t (*size)(struct VFile* vf);
				public IntPtr Sync;     //	bool (*sync)(struct VFile* vf, void* buffer, size_t size);

			}

			//---------------------------------------------------------------------
			[DllImport(NativeSymbolLocation, EntryPoint = "VFileOpen", CallingConvention = CallingConvention.Cdecl)]
			public static extern IntPtr Open(string lPath, int lFlags);

			//	struct VFile* VFileFromMemory(void* mem, size_t size);

			//	struct VFile* VFileFromConstMemory(const void* mem, size_t size);
		}

		//---------------------------------------------------------------------
		public static class Core
		{

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate bool InitFunction(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void DeinitFunction(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void DesiredVideoDimensionsFunction(IntPtr lCoreStructPtr, [Out] out uint width, [Out] out uint height);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void SetVideoBufferFunction(IntPtr lCoreStructPtr, IntPtr lBuffer, int lStride);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void ResetFunction(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void RunFrameFunction(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void RunLoopFunction(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void StepFunction(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void LoadSymbols(IntPtr lCoreStructPtr, IntPtr lVFilePtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate bool LookupIdentifier(IntPtr lCoreStructPtr, string lName, out int lValue, out int lSegment);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate IntPtr DebuggerPlatform(IntPtr lCoreStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate UInt32 BusRead8(IntPtr lCoreStructPtr, UInt32 lAddress);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate UInt32 BusRead16(IntPtr lCoreStructPtr, UInt32 lAddress);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate UInt32 BusRead32(IntPtr lCoreStructPtr, UInt32 lAddress);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void SetKeys(IntPtr lCoreStructPtr, UInt32 lKeys);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void AddKeys(IntPtr lCoreStructPtr, UInt32 lKeys);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void ClearKeys(IntPtr lCoreStructPtr, UInt32 lKeys);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void AddCoreCallbacks(IntPtr lCoreStructPtr, IntPtr lCallbackStructPtr);

			//---------------------------------------------------------------------
			[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
			public delegate void Callback(IntPtr lContext);

			//---------------------------------------------------------------------
			[StructLayout(LayoutKind.Sequential)]
			public struct Callbacks
			{
				public IntPtr Context;
				public Callback VideoFrameStarted;
				public Callback VideoFrameEnded;
				public Callback CoreCrashed;
				public Callback Sleep;
				public Callback Shutdown;
				public Callback KeysRead;
				public Callback SavedataUpdated;
			}

			//---------------------------------------------------------------------
			[StructLayout(LayoutKind.Sequential)]
			public struct DirectorySet
			{
				[MarshalAs(UnmanagedType.ByValArray, SizeConst = 260)]
				public byte[] baseName;
				public IntPtr baseDirectory;
				public IntPtr archiveDirectory;
				public IntPtr saveDirectory;
				public IntPtr patchDirectory;
				public IntPtr stateDirectory;
				public IntPtr screenshotDirectory;
				public IntPtr cheatsDirectory;
			}

			//---------------------------------------------------------------------
			[StructLayout(LayoutKind.Sequential)]
            public struct CoreStruct
            {
				public IntPtr CPU;
				public IntPtr Board;

				public IntPtr Timings;
				public IntPtr Debugger;
				public IntPtr DebuggerSymbols;

				public IntPtr VideoLogger;

				//#if !defined(MINIMAL_CORE) || MINIMAL_CORE < 2
				public DirectorySet Dirs;

				//#ifndef MINIMAL_CORE
				[MarshalAs(UnmanagedType.ByValArray, SizeConst = 12)]
				public byte[] inputMap;

				//	SizeOf(Table) = 20
				//	SizeOf(Configuration) = 40
				//	SizeOf(mCoreConfig) = 124
				[MarshalAs(UnmanagedType.ByValArray, SizeConst = 124)]
				public byte[] Config;

				//	SizeOf(mCoreOptions) = 88
				[MarshalAs(UnmanagedType.ByValArray, SizeConst = 88)]
				public byte[] opts;

				//	SizeOf(mRTCSource) = 16
				//	SizeOf(mRTCGenericSource) = 36
				[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
				public byte[] rtc;

				public InitFunction Init;   //	bool (*init)(struct mCore*);
				public DeinitFunction Deinit;   //	void (*deinit)(struct mCore*);

				//	enum mPlatform (* platform) (const struct mCore*);
				public IntPtr Platform;
				//	bool (* supportsFeature) (const struct mCore*, enum mCoreFeature);
				public IntPtr SupportsFeature;

				//	void (* setSync) (struct mCore*, struct mCoreSync*);
				public IntPtr SetSync;
				//	void (* loadConfig) (struct mCore*, const struct mCoreConfig*);
				public IntPtr LoadConfig;
				//	void (* reloadConfigOption) (struct mCore*, const char* option, const struct mCoreConfig*);
				public IntPtr ReloadConfigOption;

				//	void (* desiredVideoDimensions) (const struct mCore*, unsigned* width, unsigned* height);
				public DesiredVideoDimensionsFunction DesiredVideoDimensions;
				//	void (* setVideoBuffer) (struct mCore*, color_t* buffer, size_t stride);
				public SetVideoBufferFunction SetVideoBuffer;
				//	void (* setVideoGLTex) (struct mCore*, unsigned texid);
				public IntPtr SetVideoGLTex;

				//	void (* getPixels) (struct mCore*, const void** buffer, size_t* stride);
				public IntPtr GetPixels;
				//	void (* putPixels) (struct mCore*, const void* buffer, size_t stride);
				public IntPtr PutPixels;

				//	struct blip_t* (* getAudioChannel) (struct mCore*, int ch);
				public IntPtr GetAudioChannel;
				//	void (* setAudioBufferSize) (struct mCore*, size_t samples);
				public IntPtr SetAudioBufferSize;
				//	size_t(*getAudioBufferSize)(struct mCore*);
				public IntPtr GetAudioBufferSize;

				//	void (* addCoreCallbacks) (struct mCore*, struct mCoreCallbacks*);
				public AddCoreCallbacks AddCoreCallbacks;
				//	void (* clearCoreCallbacks) (struct mCore*);
				public IntPtr ClearCoreCallbacks;
				//	void (* setAVStream) (struct mCore*, struct mAVStream*);
				public IntPtr SetAVStream;

				//	bool (* isROM) (struct VFile* vf);
				public IntPtr IsROM;
				//	bool (* loadROM) (struct mCore*, struct VFile* vf);
				public IntPtr LoadROM;
				//	bool (* loadSave) (struct mCore*, struct VFile* vf);
				public IntPtr LoadSave;
				//	bool (* loadTemporarySave) (struct mCore*, struct VFile* vf);
				public IntPtr LoadTemporarySave;
				//	void (* unloadROM) (struct mCore*);
				public IntPtr UnloadROM;
				//	void (* checksum) (const struct mCore*, void* data, enum mCoreChecksumType type);
				public IntPtr Checksum;

				//	bool (* loadBIOS) (struct mCore*, struct VFile* vf, int biosID);
				public IntPtr LoadBIOS;
				//	bool (* selectBIOS) (struct mCore*, int biosID);
				public IntPtr SelectBIOS;

				//	bool (* loadPatch) (struct mCore*, struct VFile* vf);
				public IntPtr LoadPatch;

				//	void (* reset) (struct mCore*);
				public ResetFunction Reset;
				//	void (* runFrame) (struct mCore
				public RunFrameFunction RunFrame;
				//	void (* runLoop) (struct mCore*);
				public RunLoopFunction RunLoop;
				//	void (* step) (struct mCore
				public StepFunction Step;

				//	size_t(*stateSize)(struct mCore*);
				public IntPtr StateSize;
				//	bool (* loadState) (struct mCore*, const void* state);
				public IntPtr LoadState;
				//	bool (* saveState) (struct mCore*, void* state);
				public IntPtr SaveState;

				//	void (* setKeys) (struct mCore*, uint32_t keys);
				public SetKeys SetKeys;
				//	void (* addKeys) (struct mCore*, uint32_t keys);
				public AddKeys AddKeys;
				//	void (* clearKeys) (struct mCore*, uint32_t keys);
				public ClearKeys ClearKeys;

				//	int32_t(*frameCounter)(const struct mCore*);
				public IntPtr FrameCounter;
				//	int32_t(*frameCycles)(const struct mCore*);
				public IntPtr FrameCycles;
				//	int32_t(*frequency)(const struct mCore*);
				public IntPtr Frequency;

				//	void (* getGameTitle) (const struct mCore*, char* title);
				public IntPtr GetGameTitle;
				//	void (* getGameCode) (const struct mCore*, char* title);
				public IntPtr GetGameCode;

				//	void (* setPeripheral) (struct mCore*, int type, void*);
				public IntPtr SetPeripheral;

				//	uint32_t(*busRead8)(struct mCore*, uint32_t address);
				public BusRead8 BusRead8;
				//	uint32_t(*busRead16)(struct mCore*, uint32_t address);
				public BusRead16 BusRead16;
				//	uint32_t(*busRead32)(struct mCore*, uint32_t address);
				public BusRead32 BusRead32;

				//	void (* busWrite8) (struct mCore*, uint32_t address, uint8_t);
				public IntPtr BusWrite8;
				//	void (* busWrite16) (struct mCore*, uint32_t address, uint16_t);
				public IntPtr BusWrite16;
				//	void (* busWrite32) (struct mCore*, uint32_t address, uint32_t);
				public IntPtr BusWrite32;

				//	uint32_t(*rawRead8)(struct mCore*, uint32_t address, int segment);
				public IntPtr RawRead8;
				//	uint32_t(*rawRead16)(struct mCore*, uint32_t address, int segment);
				public IntPtr RawRead16;
				//	uint32_t(*rawRead32)(struct mCore*, uint32_t address, int segment);
				public IntPtr RawRead32;

				//	void (* rawWrite8) (struct mCore*, uint32_t address, int segment, uint8_t);
				public IntPtr RawWrite8;
				//	void (* rawWrite16) (struct mCore*, uint32_t address, int segment, uint16_t);
				public IntPtr RawWrite16;
				//	void (* rawWrite32) (struct mCore*, uint32_t address, int segment, uint32_t);
				public IntPtr RawWrite32;

				//	size_t(*listMemoryBlocks)(const struct mCore*, const struct mCoreMemoryBlock**);
				public IntPtr ListMemoryBlocks;
				//	void* (* getMemoryBlock) (struct mCore*, size_t id, size_t* sizeOut);
				public IntPtr GetMemoryBlock;

				//#ifdef USE_DEBUGGERS

				//	bool (* supportsDebuggerType) (struct mCore*, enum mDebuggerType);
				public IntPtr SupportsDebuggerType;
				//	struct mDebuggerPlatform* (* debuggerPlatform) (struct mCore*);
				public DebuggerPlatform DebuggerPlatform;
				//	struct CLIDebuggerSystem* (* cliDebuggerSystem) (struct mCore*);
				public IntPtr CLIDebuggerSystem;
				//	void (* attachDebugger) (struct mCore*, struct mDebugger*);
				public IntPtr AttachDebugger;
				//	void (* detachDebugger) (struct mCore*);
				public IntPtr DetachDebugger;

				//	void (* loadSymbols) (struct mCore*, struct VFile*);
				public LoadSymbols LoadSymbols;
				//	bool (* lookupIdentifier) (struct mCore*, const char* name, int32_t* value, int* segment);
				public LookupIdentifier LookupIdentifier;

				//#endif

				//	struct mCheatDevice* (* cheatDevice) (struct mCore*);
				public IntPtr CheatDevice;

				//size_t(*savedataClone)(struct mCore*, void** sram);
				public IntPtr SavedataClone;
				//bool (* savedataRestore) (struct mCore*, const void* sram, size_t size, bool writeback);
				public IntPtr SavedataRestore;

				//	size_t(*listVideoLayers)(const struct mCore*, const struct mCoreChannelInfo**);
				public IntPtr ListVideoLayers;
				//	size_t(*listAudioChannels)(const struct mCore*, const struct mCoreChannelInfo**);
				public IntPtr ListAudioChannels;
				//	void (* enableVideoLayer) (struct mCore*, size_t id, bool enable);
				public IntPtr EnableVideoLayer;
				//	void (* enableAudioChannel) (struct mCore*, size_t id, bool enable);
				public IntPtr EnableAudioChannel;
				//	void (* adjustVideoLayer) (struct mCore*, size_t id, int32_t x, int32_t y);
				public IntPtr AdjustVideoLayer;

				//# ifndef MINIMAL_CORE

				//	void (* startVideoLog) (struct mCore*, struct mVideoLogContext*);
				public IntPtr StartVideoLog;
				//	void (* endVideoLog) (struct mCore*);
				public IntPtr EndVideoLog;

				//#endif
			}

			//---------------------------------------------------------------------
			[DllImport(NativeSymbolLocation, EntryPoint = "mCoreFind", CallingConvention = CallingConvention.Cdecl)]
			public static extern IntPtr Find(string lPath);

            //---------------------------------------------------------------------
            [DllImport(NativeSymbolLocation, EntryPoint = "mCoreLoadFile", CallingConvention = CallingConvention.Cdecl)]
			public static extern bool LoadFile(IntPtr lCoreStructPtr, string lPath);

            //---------------------------------------------------------------------
            [DllImport(NativeSymbolLocation, EntryPoint = "mCoreConfigInit", CallingConvention = CallingConvention.Cdecl)]
			public static extern void ConfigInit(IntPtr lCoreConfigStructPtr, string lPort);

            //---------------------------------------------------------------------
            [DllImport(NativeSymbolLocation, EntryPoint = "mCoreConfigLoad", CallingConvention = CallingConvention.Cdecl)]
			public static extern bool ConfigLoad(IntPtr lCoreConfigStructPtr);

			//mCoreConfigSetDefaultValue

			//---------------------------------------------------------------------
			[DllImport(NativeSymbolLocation, EntryPoint = "mCoreLoadConfig", CallingConvention = CallingConvention.Cdecl)]
			public static extern void LoadConfig(IntPtr lCoreConfigStructPtr);

			//mCoreConfigGetIntValue

			//mCoreConfigDeinit
		}

    }

}
