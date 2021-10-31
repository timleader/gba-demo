
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
            [StructLayout(LayoutKind.Sequential)]
            public struct Logger
            {
                public LogFunction LogFunc;
                public IntPtr Filter;
            }

            //---------------------------------------------------------------------
            [DllImport(NativeSymbolLocation, EntryPoint = "mLogSetDefaultLogger", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SetDefaultLogger(IntPtr lLoggerStructPtr);

			//---------------------------------------------------------------------
			[DllImport(NativeSymbolLocation, EntryPoint = "mLog", CallingConvention = CallingConvention.Cdecl)]
			public static extern void SendLog(int lCategory, LogLevel lLevel, string lFormat, __arglist);



			/*
			 * 
						//---------------------------------------------------------------------
						[DllImport(NativeSymbolLocation, EntryPoint = "mLogFilterSet")]
						public static extern void FilterSet(IntPtr lLoggerStructPtr, string lCategory, LogLevel lLevel);
						//
						//mLogFilterTest*/

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
				public IntPtr AddCoreCallbacks;
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
				public IntPtr SetKeys;
				//	void (* addKeys) (struct mCore*, uint32_t keys);
				public IntPtr AddKeys;
				//	void (* clearKeys) (struct mCore*, uint32_t keys);
				public IntPtr ClearKeys;

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
				public IntPtr BusRead8;
				//	uint32_t(*busRead16)(struct mCore*, uint32_t address);
				public IntPtr BusRead16;
				//	uint32_t(*busRead32)(struct mCore*, uint32_t address);
				public IntPtr BusRead32;

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
				public IntPtr DebuggerPlatform;
				//	struct CLIDebuggerSystem* (* cliDebuggerSystem) (struct mCore*);
				public IntPtr CLIDebuggerSystem;
				//	void (* attachDebugger) (struct mCore*, struct mDebugger*);
				public IntPtr AttachDebugger;
				//	void (* detachDebugger) (struct mCore*);
				public IntPtr DetachDebugger;

				//	void (* loadSymbols) (struct mCore*, struct VFile*);
				public IntPtr LoadSymbols;
				//	bool (* lookupIdentifier) (struct mCore*, const char* name, int32_t* value, int* segment);
				public IntPtr LookupIdentifier;

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
