using System;

using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml.Serialization;
using System.Diagnostics;
namespace TorrentF.Utilities
{
    [Serializable]
    public class TorrentFConfig
    {
        public string serializationFileName = @".\SD Card\TorrentFConfig.xml";
        
        public int uploadingServerPort = 2620;
        
        public string trackerIp = "127.0.0.1";
        
        public int trackerHttpPort = 2617;
        
        public string bitHocClientRelativePath = @".\BitHoc Client.exe";
        
        // A set of private caracters used by the tracker and TorrentF
        public char[] privateCharacters = { '*', '#', '\n' };
        public bool IsPrivateCharacter(char x)
        {
            bool res = false;
            foreach (char c in privateCharacters)
            {
                if (c == x)
                {
                    res = true;
                    break;
                }
            }
            return res;
        }

        // Attributes related to the RemoteDhtCall API
        public string xmlRpcServiceUrl = "http://194.254.174.173:3640/";
        public int xmlRequestTTL = 3600;
        public string applicationName = "BitHocSE";
        
    
        // Indicates whether the user is going to use the remote DHT service or the local tracker
        public bool useLocalTracker = true; 
        
        private static TorrentFConfig _config = null;
        private void CopyTorrentFConfig(ref TorrentFConfig c)
        {
            serializationFileName = c.serializationFileName;
            uploadingServerPort = c.uploadingServerPort;
            trackerIp = c.trackerIp;
            trackerHttpPort = c.trackerHttpPort;
            privateCharacters = c.privateCharacters;
        }
        private TorrentFConfig()
        {
            
        }

        public static TorrentFConfig GetConfig()
        {
            if (_config == null)
            {
                _config = new TorrentFConfig();
                TorrentFConfig tfc = _config.DeserializeConfiguration();
                if (tfc != null)
                {
                    // we make a copy of that object
                    Trace.Assert(tfc.privateCharacters.Length > 0, "No private characters were specified.");
                    Trace.Assert(tfc.trackerHttpPort > 1024, "Invalid Tracker Http Port.");
                    Trace.Assert(tfc.trackerIp.Length > 0, "Invalid Tracker ip.");
                    Trace.Assert(tfc.uploadingServerPort > 1024, "Invalid Uploading Server Port.");
                    Trace.Assert(tfc.bitHocClientRelativePath.Length > 0, "Relative path to the BitHoc Client.");
                    Trace.Assert(tfc.applicationName.Length > 0, "Invalid application name.");
                    _config.CopyTorrentFConfig(ref tfc);
                }
                else
                {
                    // Serialize the current object file
                    _config.SerializeConfiguation();
                }
                return _config;
            }
            else
            {
                
                return _config;
            }
        }

        // XML Serialization function will be called when this object is destroyed
        public void SerializeConfiguation()
        {
            FileStream fs = new FileStream(serializationFileName, FileMode.Create);
            XmlSerializer xs = new XmlSerializer(typeof(TorrentFConfig));
            xs.Serialize(fs, this);
            fs.Flush();
            fs.Close();
        }
        
        // Each time a configuration object is created, we verify if we can load a serialized
        // xml file or not
        public TorrentFConfig DeserializeConfiguration()
        {
            FileInfo fi = new FileInfo(serializationFileName);
            if (fi.Exists)
            {
                FileStream fs = new FileStream(serializationFileName, FileMode.Open, FileAccess.Read);
                XmlSerializer xs = new XmlSerializer(typeof(TorrentFConfig));
                TorrentFConfig tmp = (TorrentFConfig)xs.Deserialize(fs);
                fs.Close();
                return tmp;
            }
            else return null;
        }


    }
}
