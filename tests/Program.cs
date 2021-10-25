
using System;
using System.Runtime.InteropServices;

namespace GBA.Tests.source
{

    public static class Program
    {

        //---------------------------------------------------------------------
        private static void Log(IntPtr lLoggerStructPtr, int lCategory, mGBA.Log.LogLevel lLevel, string lFormat, IntPtr lArgs)
        {
            Console.Out.WriteLine(lFormat);

            //  errors are bad 
        }

        //---------------------------------------------------------------------
        public unsafe static void Main(string[] lArgs)
        {
            mGBA.Log.Logger lLogger = new mGBA.Log.Logger { LogFunc = Log };
            IntPtr lLoggerPtr = Marshal.AllocHGlobal(Marshal.SizeOf(lLogger));
            Marshal.StructureToPtr(lLogger, lLoggerPtr, false);

            mGBA.Log.SetDefaultLogger(lLoggerPtr);


            IntPtr lCoreStructPtr = mGBA.Core.Find("G:/workspace/git/gba-demo/gba-demo.gba");
            mGBA.Core.CoreStruct lCore = Marshal.PtrToStructure<mGBA.Core.CoreStruct>(lCoreStructPtr);

            bool lResult = lCore.Init(lCoreStructPtr); 
            if (!lResult)
            {
                Console.Out.WriteLine("SHIT");
            }

            uint lWidth, lHeight;
            lCore.DesiredVideoDimensions(lCoreStructPtr, out lWidth, out lHeight);

            lResult = mGBA.Core.LoadFile(lCoreStructPtr, "G:/workspace/git/gba-demo/gba-demo.gba");
            if (!lResult)
            {
                Console.Out.WriteLine("SHIT");
            }

            while (true)
            {
                //  setKeys 

                //  runFrame

                //  getVideoOutput 
            }

        }

    }

}
