using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Linq;


namespace MyInstallCustomAction
{
    [RunInstaller(true)]
    public partial class BitHocSearchEngineInstaller : Installer
    {
        public BitHocSearchEngineInstaller()
        {
            InitializeComponent();
        }
        public override void Commit(IDictionary savedState)
        {
            // Call the Commit method of the base class
            base.Commit(savedState);
            // Open the registry key containing the path to the Application Manager
            Microsoft.Win32.RegistryKey key = null;
            key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey("Software\\microsoft\\windows\\currentversion\\app paths\\ceappmgr.exe");

            // If the key is not null, then ActiveSync is installed on the user's desktop computer
            if (key != null)
            {
                // Get the path to the Application Manager from the registry value
                string appPath = null;
                appPath = key.GetValue(null).ToString();

                string strIniFilePath = "\"" + Context.Parameters["targetdir"] + "BitHocSearchEngineSetup.ini\"";

                if (appPath != null)
                {
                    // Now launch the Application Manager
                    System.Diagnostics.Process process = new System.Diagnostics.Process();
                    process.StartInfo.FileName = appPath;
                    process.StartInfo.Arguments = strIniFilePath;
                    process.Start();
                }
            }
            else
            {
                // No Active Sync - throw a message

            }
        }
    }

}
