
using System.IO;

using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

using NUnit.Framework;

namespace GBA.Tests
{

    public class DemoTests
    {
        //---------------------------------------------------------------------
        private GBAEmulator mGBAEmulator;

        //---------------------------------------------------------------------
        public class TestConfiguration
        {
            public string RomPath;
        }

        //---------------------------------------------------------------------
        [SetUp]
        public void Setup()
        {
            var lYamlDeserializer = new DeserializerBuilder()
               .WithNamingConvention(UnderscoredNamingConvention.Instance) 
               .Build();

            var lConfiguration = lYamlDeserializer.Deserialize<TestConfiguration>(File.ReadAllText("configuration.yml"));

            mGBAEmulator = new GBAEmulator(lConfiguration.RomPath);
        }

        //---------------------------------------------------------------------
        [Test]
        public void RunToStateTitle_English()
        {
            //  Reset the game for a clean test 
            mGBAEmulator.Reset();

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_splash", 100);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_lang_sel", 200);

            Assert.That(mGBAEmulator.ReadDebugVariableUInt8("selected_idx") == 0);

            mGBAEmulator.PressAndReleaseButton(GBAEmulator.Button.A);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_title", 200);
        }

        //---------------------------------------------------------------------
        [Test]
        public void RunToStateTitle_French()
        {
            //  Reset the game for a clean test 
            mGBAEmulator.Reset();

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_splash", 100);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_lang_sel", 200);

            Assert.That(mGBAEmulator.ReadDebugVariableUInt8("selected_idx") == 0);

            mGBAEmulator.PressAndReleaseButton(GBAEmulator.Button.DOWN);

            Assert.That(mGBAEmulator.ReadDebugVariableUInt8("selected_idx") == 1);

            mGBAEmulator.PressAndReleaseButton(GBAEmulator.Button.A);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_title", 200);
        }

        //---------------------------------------------------------------------
        [Test]
        public void RunToNewGameFlow()
        {
            RunToStateTitle_English();

            Assert.That(mGBAEmulator.ReadDebugVariableUInt8("selected_idx") == 0);

            mGBAEmulator.PressAndReleaseButton(GBAEmulator.Button.A);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_terminal", 200);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_video", 2000);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_dialogue", 4000);
        }

        //---------------------------------------------------------------------
        [Test]
        public void RunToLoadGame()
        {
            RunToStateTitle_English();

            Assert.That(mGBAEmulator.ReadDebugVariableUInt8("selected_idx") == 0);

            mGBAEmulator.PressAndReleaseButton(GBAEmulator.Button.DOWN);

            Assert.That(mGBAEmulator.ReadDebugVariableUInt8("selected_idx") == 1);

            mGBAEmulator.PressAndReleaseButton(GBAEmulator.Button.A);

            mGBAEmulator.RunUntil(() => mGBAEmulator.ReadDebugVariableString("state_name") == "st_savegame", 200);
        }
    }
}
