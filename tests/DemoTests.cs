
using System;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

using NUnit.Framework;

namespace GBA.Tests
{
    public class DemoTests
    {

        //---------------------------------------------------------------------
        public struct DebugVar
        {
            public string Name;
            public UInt32 Address; 
        }

        //---------------------------------------------------------------------
        public Dictionary<string, DebugVar> mDebugVarList = new Dictionary<string, DebugVar>();

        //---------------------------------------------------------------------
        public void DebugVariable(UInt16 lFlags, string lName, UInt32 lAddress)
        {
            if ((lFlags & 0x01) > 0)
            {
                mDebugVarList[lName] = new DebugVar { Name = lName, Address = lAddress };
            }
            else
            {
                mDebugVarList.Remove(lName);
            }
        }

        //---------------------------------------------------------------------
        public byte? ReadDebugVariableUInt8(string lName)
        {
            byte? lResult = null;
            if (mDebugVarList.ContainsKey(lName))
            {
                uint lAddress = mDebugVarList[lName].Address;
                lResult = (byte)mGBACore.BusRead8(mGBACorePtr, lAddress);
            }
            return lResult;
        }

        //---------------------------------------------------------------------
        public ushort? ReadDebugVariableUInt16(string lName)
        {
            ushort? lResult = null;
            if (mDebugVarList.ContainsKey(lName))
            {
                uint lAddress = mDebugVarList[lName].Address;
                lResult = (ushort)mGBACore.BusRead16(mGBACorePtr, lAddress);
            }
            return lResult;
        }

        //---------------------------------------------------------------------
        public uint? ReadDebugVariableUInt32(string lName)
        {
            uint? lResult = null;
            if (mDebugVarList.ContainsKey(lName))
            {
                uint lAddress = mDebugVarList[lName].Address;
                lResult = (uint)mGBACore.BusRead32(mGBACorePtr, lAddress);
            }
            return lResult;
        }

        //---------------------------------------------------------------------
        public string ReadDebugVariableString(string lName)
        {
            string lResult = null;
            if (mDebugVarList.ContainsKey(lName))
            {
                uint lAddress = mDebugVarList[lName].Address;
                byte[] lDataChunk = new byte[256];

                int lDataChunkIdx = 0;
                byte lValue;
                do
                {
                    lValue = (byte)mGBACore.BusRead8(mGBACorePtr, lAddress++);
                    lDataChunk[lDataChunkIdx++] = lValue;

                } while (lValue != 0);


                lResult = Encoding.ASCII.GetString(lDataChunk, 0, lDataChunkIdx - 1);
            }
            return lResult;
        }


        //---------------------------------------------------------------------
        [DllImport("msvcrt.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern int vsprintf([MarshalAs(UnmanagedType.LPStr)] StringBuilder lOutput, string format, IntPtr args);

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

        //---------------------------------------------------------------------
        private void CoreCrashed(IntPtr lContext)
        {
            Assert.Fail("CoreCrashed");
        }

        //---------------------------------------------------------------------
        public class TestConfiguration
        {
            public string RomPath;
        }

        //---------------------------------------------------------------------
        private mGBA.Core.CoreStruct mGBACore;
        private IntPtr mGBACorePtr;
        private mGBA.Log.Logger mGBALogger;

        //---------------------------------------------------------------------
        [SetUp]
        public void Setup()
        {
            var lYamlDeserializer = new DeserializerBuilder()
               .WithNamingConvention(UnderscoredNamingConvention.Instance)  // see height_in_inches in sample yml 
               .Build();

            var lConfiguration = lYamlDeserializer.Deserialize<TestConfiguration>(File.ReadAllText("configuration.yml"));

            var lRomPath = lConfiguration.RomPath;

            //  Logging
            mGBALogger = new mGBA.Log.Logger   //  I think this is moving
            {
                LogFunc = Log,
                DebugVarsFunc = DebugVariable
            };

            IntPtr lLoggerPtr = Marshal.AllocHGlobal(Marshal.SizeOf(mGBALogger));
            Marshal.StructureToPtr(mGBALogger, lLoggerPtr, false);

            mGBA.Log.SetDefaultLogger(lLoggerPtr);


            mGBACorePtr = mGBA.Core.Find(lRomPath);
            mGBACore = Marshal.PtrToStructure<mGBA.Core.CoreStruct>(mGBACorePtr);

            bool lResult = mGBACore.Init(mGBACorePtr);
            if (!lResult)
            {
                Console.Out.WriteLine("SHIT");
            }

            uint lWidth, lHeight;
            mGBACore.DesiredVideoDimensions(mGBACorePtr, out lWidth, out lHeight);

            //IntPtr lVideoOutputBuffer = Marshal.AllocHGlobal((int)(lWidth * lHeight * 4));
            //lCore.SetVideoBuffer(lCoreStructPtr, lVideoOutputBuffer, (int)(lWidth * 4));

            lResult = mGBA.Core.LoadFile(mGBACorePtr, lRomPath);
            if (!lResult)
            {
                Console.Out.WriteLine("SHIT");
            }

            IntPtr lConfigPtr = new IntPtr(mGBACorePtr.ToInt64() + Marshal.OffsetOf<mGBA.Core.CoreStruct>("Config").ToInt64());

            mGBA.Core.ConfigInit(lConfigPtr, "test");
            mGBA.Core.ConfigLoad(lConfigPtr);

            //applyArguments(args, NULL, &core->config);
            //mCoreConfigSetDefaultValue(&core->config, "idleOptimization", "detect");

            mGBA.Core.LoadConfig(mGBACorePtr);


            //  Callbacks
            mGBA.Core.Callbacks lCallbacks = new mGBA.Core.Callbacks
            {
                CoreCrashed = CoreCrashed
            };

            IntPtr lCallbacksPtr = Marshal.AllocHGlobal(Marshal.SizeOf(lCallbacks));
            Marshal.StructureToPtr(lCallbacks, lCallbacksPtr, false);

            mGBACore.AddCoreCallbacks(mGBACorePtr, lCallbacksPtr);

            //mCoreConfigGetIntValue(&core->config, "logLevel", &_logLevel);

            //mGBA.Log.SendLog(0, mGBA.Log.LogLevel.WARN, "Test Log", __arglist());

            //  fuck that, just go with a fixed address 
        }

        //---------------------------------------------------------------------
        [TearDown]
        public void TearDown()
        {
            //mCoreConfigDeinit(&core->config);
            mGBACore.Deinit(mGBACorePtr);
        }

        //---------------------------------------------------------------------
        private void RunUntil(Func<bool> lCheckFunc, int lMaxNumberOfFrames = int.MaxValue)
        {
            int lFrame = 0;
            while (lCheckFunc() == false)
            {
                mGBACore.RunFrame(mGBACorePtr);
                lFrame++;

                if (lFrame > lMaxNumberOfFrames)
                    Assert.Fail();
            }
        }

        //---------------------------------------------------------------------
        private enum Button
        {
            A       = (1 << 0),
            B       = (1 << 1),
            SELECT  = (1 << 2),
            START   = (1 << 3),
            RIGHT   = (1 << 4),
            LEFT    = (1 << 5),
            UP      = (1 << 6),
            DOWN    = (1 << 7),
            R       = (1 << 8),
            L       = (1 << 9)
        }

        //---------------------------------------------------------------------
        private void PressAndReleaseButton(Button lInput)
        {
            uint lKeys = (uint)lInput;
            mGBACore.SetKeys(mGBACorePtr, lKeys);

            mGBACore.RunFrame(mGBACorePtr);

            lKeys = 0;
            mGBACore.SetKeys(mGBACorePtr, lKeys);

            mGBACore.RunFrame(mGBACorePtr);
        }

        //  GC.Collect will kill the test 

        //---------------------------------------------------------------------
        [Test]
        public void RunToStateTitle_English()
        {
            //  Reset the game for a clean test 
            mGBACore.Reset(mGBACorePtr);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_splash", 100);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_lang_sel", 200);

            Assert.That(ReadDebugVariableUInt8("selected_idx") == 0);

            PressAndReleaseButton(Button.A);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_title", 200);
        }

        //---------------------------------------------------------------------
        [Test]
        public void RunToStateTitle_French()
        {
            //  Reset the game for a clean test 
            mGBACore.Reset(mGBACorePtr);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_splash", 100);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_lang_sel", 200);

            Assert.That(ReadDebugVariableUInt8("selected_idx") == 0);

            PressAndReleaseButton(Button.DOWN);

            Assert.That(ReadDebugVariableUInt8("selected_idx") == 1);

            PressAndReleaseButton(Button.A);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_title", 200);
        }

        //---------------------------------------------------------------------
        [Test]
        public void RunToNewGameFlow()
        {
            RunToStateTitle_English();

            Assert.That(ReadDebugVariableUInt8("selected_idx") == 0);

            PressAndReleaseButton(Button.A);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_terminal", 200);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_video", 2000);

            RunUntil(() => ReadDebugVariableString("state_name") == "st_dialogue", 4000);
        }
    }
}
