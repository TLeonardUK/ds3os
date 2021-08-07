// Dark Souls 3 - Open Server

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Loader
{
    public class PatchingUtils
    {
        // The TEA encryption key used to encrypt the server info block.
        public static uint[] ServerInfoTEAEncryptionKey = {
            0x4B694CD6,
            0x96ADA235,
            0xEC91D9D4,
            0x23F562E5
        };

        // Maximum size our server info block can be. Technically there is some extra space after this
        // as FROM seem to included a bunch of their stack memory when encrypting this lol. But it 
        // feels safest not to stray into that data if we can avoid it.
        public static int ServerInfoPatchSize = 520;

        // Maximum length of the UTF8 encoded public key. 
        public static int ServerInfoMaxKeySize = 430;

        // Maximum length of the UTF16 encoded hostname.
        public static int ServerInfoMaxHostSize = 85; // Leave at least 2 bytes from the end of ServerInfoPatchSize for nullptr.

        // Offset into the data block that the hostname is placed.
        public static int ServerInfoHostOffset = 432;

        // Makes the TEA encrypted data block that contains the hostname and public key of 
        // the login server. This can be taken and the one in memory replaced so we can connect
        // to the new server.
        public static byte[] MakeEncryptedServerInfo(string server_hostname, string public_key)
        {
            byte[] data_block = new byte[ServerInfoPatchSize];

            byte[] key_bytes = Encoding.UTF8.GetBytes(public_key);
            byte[] host_bytes = Encoding.Unicode.GetBytes(server_hostname);

            if (key_bytes.Length > ServerInfoMaxKeySize)
            {
                return null;
            }

            if (host_bytes.Length > ServerInfoMaxHostSize)
            {
                return null;
            }

            Array.Copy(key_bytes, 0, data_block, 0, key_bytes.Length);
            Array.Copy(host_bytes, 0, data_block, ServerInfoHostOffset, host_bytes.Length);

            return EncryptionUtils.Tea32Encrypt(data_block, ServerInfoTEAEncryptionKey);
        }
    }
}
