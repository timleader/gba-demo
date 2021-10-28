
using System;
using System.Text;
using System.Runtime.InteropServices;

using NUnit.Framework;

namespace GBA.Tests
{
    public class Tests
    {

        //---------------------------------------------------------------------
        [DllImport("msvcrt.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern int vsprintf(
           [MarshalAs(UnmanagedType.LPStr)] StringBuilder lOutput,
            string format,
            IntPtr args);

        //---------------------------------------------------------------------
        private void Log(IntPtr lLoggerStructPtr, int lCategory, mGBA.Log.LogLevel lLevel, string lFormat, IntPtr lArgs)
        {
            var lBuffer = new StringBuilder(1024); 
            var lLength = vsprintf(lBuffer, lFormat, lArgs);
            //  maybe call C code to format the string ... 

            if (lLength > 0)
            {
                var lOutput = lBuffer.ToString();
                switch (lLevel)
                {
                    case mGBA.Log.LogLevel.FATAL:
                    case mGBA.Log.LogLevel.ERROR:
                    case mGBA.Log.LogLevel.GAME_ERROR:
                        Assert.Fail(lOutput);
                        break;
                }

                Console.Out.WriteLine(lOutput);
            }


            //  errors are bad 
            
            //  detect system logs, like enterirng a gamestate, 

        }


        /*
         * Navigate through game 
         *  Launch
         *  Wait for Language Select
         *  Press A
         *  Wait for Title
         *  Press A
         *  Wait for Play -- and sequence finished 
         *  Pause Game
         *  Save Game
         *  Exit 
         *  Load Game
         *  
         * Visual Comparison
         *  
         *  
         * 
         */

        //---------------------------------------------------------------------


        //---------------------------------------------------------------------
        [SetUp]
        public void Setup()
        {
        }

        //---------------------------------------------------------------------
        [Test]
        public void Test1()
        {
            var lRomPath = TestContext.Parameters.Get("rom", "gba-demo.gba");

            //  pre-allocate a buffer 

            mGBA.Log.Logger lLogger = new mGBA.Log.Logger { LogFunc = Log };
            IntPtr lLoggerPtr = Marshal.AllocHGlobal(Marshal.SizeOf(lLogger));
            Marshal.StructureToPtr(lLogger, lLoggerPtr, false);

            mGBA.Log.SetDefaultLogger(lLoggerPtr);


            IntPtr lCoreStructPtr = mGBA.Core.Find(lRomPath);
            mGBA.Core.CoreStruct lCore = Marshal.PtrToStructure<mGBA.Core.CoreStruct>(lCoreStructPtr);

            bool lResult = lCore.Init(lCoreStructPtr);
            if (!lResult)
            {
                Console.Out.WriteLine("SHIT");
            }

            uint lWidth, lHeight;
            lCore.DesiredVideoDimensions(lCoreStructPtr, out lWidth, out lHeight);

            //IntPtr lVideoOutputBuffer = Marshal.AllocHGlobal((int)(lWidth * lHeight * 4));
            //lCore.SetVideoBuffer(lCoreStructPtr, lVideoOutputBuffer, (int)(lWidth * 4));

            lResult = mGBA.Core.LoadFile(lCoreStructPtr, lRomPath);
            if (!lResult)
            {
                Console.Out.WriteLine("SHIT");
            }

            IntPtr lConfigPtr = new IntPtr(lCoreStructPtr.ToInt64() + Marshal.OffsetOf<mGBA.Core.CoreStruct>("Config").ToInt64());

            mGBA.Core.ConfigInit(lConfigPtr, "test");
            mGBA.Core.ConfigLoad(lConfigPtr); 

            //applyArguments(args, NULL, &core->config);
            //mCoreConfigSetDefaultValue(&core->config, "idleOptimization", "detect");

            mGBA.Core.LoadConfig(lCoreStructPtr);

            //mCoreConfigGetIntValue(&core->config, "logLevel", &_logLevel);

            //mGBA.Log.SendLog(0, mGBA.Log.LogLevel.WARN, "Test Log", __arglist());

            lCore.Reset(lCoreStructPtr);       

            for (int lIdx = 0; lIdx < 2000; ++lIdx)
            {
                //  setKeys 

                //  runFrame
                lCore.RunFrame(lCoreStructPtr);

                //  getVideoOutput 
            }

            //mCoreConfigDeinit(&core->config);
            //core->deinit(core);
        }
    }
}