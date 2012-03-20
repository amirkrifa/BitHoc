using System;

using System.Collections.Generic;
using System.Text;

namespace TorrentF.FilesStatus
{
    public class SharingSessionDetails
    {
        private int numberOfLeechers = 0;
        public int NumberOfLeechers
        {
            get
            {
                int tmp = 0;
                lock (this)
                {
                    tmp = numberOfLeechers;
                }
                return tmp;
            }
            set
            {
                lock (this)
                {
                    numberOfLeechers = value;
                }
            }
        }

        private int numberOfSeeders = 0;
        public int NumberOfSeerders
        {
            get
            {
                int tmp = 0;
                lock (this)
                {
                    tmp = numberOfSeeders;
                }
                return tmp;
            }
            set
            {
                lock (this)
                {
                    numberOfSeeders = value;
                }
            }
        }

        public int NumberOfInvolvedPeers
        {
            get
            {
                int tmp = 0;
                lock (this)
                {
                    tmp = numberOfLeechers + numberOfSeeders;
                }
                return tmp;
            }
         }

        // List used to store the list of peers Ips involved in the sharing session
        List<string> listOfPeers = null;
        public List<string> ListOfPeers
        {
            get
            {
                List<string> tmp = null;
                lock (this)
                {
                    tmp = listOfPeers;
                }
                return tmp;
            }
        }

        public SharingSessionDetails()
        {
            listOfPeers = new List<string>();
        }

    }
}
