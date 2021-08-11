/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace Loader
{
    public partial class MainForm : Form
    {
        private ServerConfigList ServerList = new ServerConfigList();
        private IntPtr RunningProcessHandle = IntPtr.Zero;

        public MainForm()
        {
            InitializeComponent();
        }

        private void SaveConfig()
        {
            ProgramSettings.Default.exe_location = ExeLocationTextBox.Text;
            ProgramSettings.Default.server_config_json = ServerList.ToJson();

            ProgramSettings.Default.Save();
        }

        private void ValidateUI()
        {
            bool LaunchEnabled = true;

            DarkSoulsLoadConfig LoadConfig;

            if (!File.Exists(ExeLocationTextBox.Text))
            {
                ExeLocationTextBox.BackColor = System.Drawing.Color.Pink;
                BuildInfoLabel.Text = ExeLocationTextBox.Text.Length > 0 ? "Executable does not exist" : "";
                BuildInfoLabel.ForeColor = System.Drawing.Color.Red;
                LaunchEnabled = false;
            }
            else if (!BuildConfig.ExeLoadConfiguration.TryGetValue(ExeUtils.GetExeSimpleHash(ExeLocationTextBox.Text), out LoadConfig))
            {
                ExeLocationTextBox.BackColor = System.Drawing.Color.Pink;
                BuildInfoLabel.Text = "Executable not a recognised version";
                BuildInfoLabel.ForeColor = System.Drawing.Color.Red;
                LaunchEnabled = false;
            }
            else
            {
                BuildInfoLabel.Text = "Recognised as " + LoadConfig.VersionName;
                BuildInfoLabel.ForeColor = System.Drawing.Color.Black;

                ExeLocationTextBox.BackColor = System.Drawing.SystemColors.Control;
            }

            if (ImportedServerListView.SelectedItems.Count <= 0)
            {
                LaunchEnabled = false;
                RemoveButton.Enabled = false;
            }
            else
            {
                RemoveButton.Enabled = true;
            }

            if (RunningProcessHandle != IntPtr.Zero)
            {
                LaunchEnabled = false;
                LaunchButton.Text = "Running ...";
            }
            else
            {
                LaunchButton.Text = "Launch Game";
            }

            LaunchButton.Enabled = LaunchEnabled;
        }

        private void BuildServerList()
        {
            ImportedServerListView.Items.Clear();

            foreach (ServerConfig Config in ServerList.Servers)
            {
                ListViewItem Item = new ListViewItem(new string[] {
                    Config.Name,
                    Config.Hostname,
                    Config.Description
                });
                ImportedServerListView.Items.Add(Item);
            }
        }

        private void OnLoaded(object sender, EventArgs e)
        {
            ExeLocationTextBox.Text = ProgramSettings.Default.exe_location;
            ServerConfigList.FromJson(ProgramSettings.Default.server_config_json, out ServerList);

            ValidateUI();
            BuildServerList();

            // Debugging experiments.
            
            /*
            byte[] bytes = File.ReadAllBytes(@"Y:\DS3Server\Research\server_info.bin");
            byte[] decrypted_bytes = EncryptionUtils.Tea32Decrypt(bytes, PatchingUtils.ServerInfoTEAEncryptionKey);
            byte[] encrypted_bytes = EncryptionUtils.Tea32Encrypt(decrypted_bytes, PatchingUtils.ServerInfoTEAEncryptionKey);
            for (int i = 0; i < decrypted_bytes.Length; i++)
            {
                Debug.Assert(bytes[i] == encrypted_bytes[i]);
            }

            File.WriteAllBytes(@"Y:\DS3Server\Research\server_info.bin", EncryptionUtils.temp_test);
            byte[] bytes = File.ReadAllBytes(@"Y:\DS3Server\Research\server_info.bin");
            byte[] decrypted_bytes = EncryptionUtils.Tea32Decrypt(bytes, PatchingUtils.ServerInfoTEAEncryptionKey);
            File.WriteAllBytes(@"Y:\DS3Server\Research\server_info.decrypted.bin", decrypted_bytes);
            */
        }

        private void OnBrowseForExecutable(object sender, EventArgs e)
        {
            using (OpenFileDialog Dialog = new OpenFileDialog())
            {
                Dialog.Filter = "Dark Souls III|DarkSoulsIII.exe|All Files|*.*";
                Dialog.Title = "Select DS3 Executable Location";

                if (Dialog.ShowDialog() == DialogResult.OK)
                {
                    ExeLocationTextBox.Text = Dialog.FileName;

                    SaveConfig();
                    ValidateUI();
                }
            }
        }

        private void OnImportServerConfig(object sender, EventArgs e)
        {
            using (OpenFileDialog Dialog = new OpenFileDialog())
            {
                Dialog.Filter = "Dark Souls III - Server Config|*.ds3osconfig|All Files|*.*";
                Dialog.Title = "Select Server Configuration File";

                if (Dialog.ShowDialog() == DialogResult.OK)
                {
                    string JsonContents = File.ReadAllText(Dialog.FileName);
                    ServerConfig NewServerConfig;

                    if (!ServerConfig.FromJson(JsonContents, out NewServerConfig))
                    {
                        MessageBox.Show("Failed to load server configuration, are you sure its in the correct format?", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                    {
                        ServerList.Servers.Add(NewServerConfig);
                    }

                    BuildServerList();
                    SaveConfig();
                    ValidateUI();
                }
            }
        }

        private void OnSelectedServerChanged(object sender, EventArgs e)
        {
            ValidateUI();
        }

        private void OnRemoveClicked(object sender, EventArgs e)
        {
            if (ImportedServerListView.SelectedItems.Count > 0)
            {
                ServerList.Servers.RemoveAt(ImportedServerListView.SelectedIndices[0]);

                BuildServerList();
                SaveConfig();
                ValidateUI();
            }
        }

        private void OnLaunch(object sender, EventArgs e)
        {
            ServerConfig Config = ServerList.Servers[ImportedServerListView.SelectedIndices[0]];

            byte[] DataBlock = PatchingUtils.MakeEncryptedServerInfo(Config.Hostname, Config.PublicKey);
            if (DataBlock == null)
            {
                MessageBox.Show("Failed to encode server info patch. Potentially server information is too long to fit into the space available.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            DarkSoulsLoadConfig LoadConfig;
            if (!BuildConfig.ExeLoadConfiguration.TryGetValue(ExeUtils.GetExeSimpleHash(ExeLocationTextBox.Text), out LoadConfig))
            {
                MessageBox.Show("Failed to determine exe version, unable to patch.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string ExeLocation = ExeLocationTextBox.Text;
            string ExeDirectory = Path.GetDirectoryName(ExeLocation);

            string AppIdFile = Path.Combine(ExeDirectory, "steam_appid.txt");
            if (!File.Exists(AppIdFile))
            {
                File.WriteAllText(AppIdFile, BuildConfig.SteamAppId.ToString());
            }

            STARTUPINFO StartupInfo = new STARTUPINFO();
            PROCESS_INFORMATION ProcessInfo = new PROCESS_INFORMATION();

            bool Result = WinAPI.CreateProcess(
                null,
                ExeLocation,
                IntPtr.Zero,
                IntPtr.Zero,
                false,
                ProcessCreationFlags.CREATE_SUSPENDED,
                IntPtr.Zero,
                ExeDirectory,
                ref StartupInfo,
                out ProcessInfo
            );

            if (!Result)
            {
                MessageBox.Show("Failed to launch data souls 3 executable.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            int BytesWritten;
            bool WriteSuccessful = WinAPI.WriteProcessMemory(ProcessInfo.hProcess, (IntPtr)LoadConfig.ServerInfoAddress, DataBlock, (uint)DataBlock.Length, out BytesWritten);
            if (!WriteSuccessful || BytesWritten != DataBlock.Length)
            {
                MessageBox.Show("Failed to write full patch to memory. Game may or may not work.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }

            WinAPI.ResumeThread(ProcessInfo.hThread);

            RunningProcessHandle = ProcessInfo.hProcess;
            RunningProcessUpdateTimer.Enabled = true;

            ValidateUI();
        }

        private void OnProcessUpdateTimer(object sender, EventArgs e)
        {
            uint ExitCode = 0;
            if (!WinAPI.GetExitCodeProcess(RunningProcessHandle, out ExitCode) || ExitCode != (uint)ProcessExitCodes.STILL_ACTIVE)
            {
                RunningProcessHandle = IntPtr.Zero;
                RunningProcessUpdateTimer.Enabled = false;
            }

            ValidateUI();
        }
    }
}
