// Dark Souls 3 - Open Server

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Loader
{
    public static class ExeUtils
    {
        // Makes a hash from information about an executable. We do this rather than doing a full
        // exe hash just for the sake of quick calculation.
        public static string MakeSimpleExeHash(string ExeVersion, long ExeSize)
        {
            return ExeVersion + "|" + ExeSize;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="FilePath"></param>
        /// <returns></returns>
        public static string GetExeSimpleHash(string FilePath)
        {
            FileInfo info = new FileInfo(FilePath);
            FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(FilePath);

            return MakeSimpleExeHash(versionInfo.FileVersion, info.Length);
        }
    }
}
