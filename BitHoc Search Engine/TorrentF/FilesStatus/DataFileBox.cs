using System;

using System.Collections.Generic;
using System.Text;
using System.IO;

namespace TorrentF.FilesStatus
{
    // This class will be used as a box to encapsulate the data file and its related 
    // attributes, so we can serialize (binary) the box and send it to the remote
    // host
    [Serializable]
    public class DataFileBox
    {
        
        private bool inversed = false;

        public string finalData;

        public string finalDescription;

        public DataFileBox()
        {

        }
        public void BoxDataFile(string absolutePath)
        {
            StringBuilder data = new StringBuilder();
            FileStream fs = new FileStream(absolutePath, FileMode.Open, FileAccess.Read);
            byte[] tmp = new byte[1024];
            int s = 0;
            while ((s = fs.Read(tmp,0,1024)) != 0 )
            {
                data.Append(System.Text.Encoding.ASCII.GetString(tmp, 0, s));
            }
            fs.Close();
            finalData = data.ToString();
        }

        public void BoxDataFileDescription(string desc)
        {
            StringBuilder description = new StringBuilder();
            description.Append(desc);
            finalDescription = description.ToString();
        }

        public int WriteDataFileTo(string absolutePath)
        {
            FileStream fs = new FileStream(absolutePath, FileMode.OpenOrCreate, FileAccess.Write);
            if (!inversed)
            {
                inversed = true;
            }
            
            byte[] tmp = System.Text.Encoding.ASCII.GetBytes(finalData.ToCharArray());
            fs.Write(tmp, 0, tmp.Length);
            fs.Flush();
            fs.Close();
            return finalData.Length;
        }

        public string GetDataFileDescription()
        {
            if (!inversed)
            {
                inversed = true;
            }

            return finalDescription;
        }
        public string GetData()
        {
            if (!inversed)
            {
                inversed = true;
            }
            return finalData;
        }
    }
}
