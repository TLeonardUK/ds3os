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
using System.Diagnostics;
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
        private Task QueryServerTask = null;

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

            if (!SteamUtils.IsSteamRunningAndLoggedIn())
            {
                LaunchEnabled = false;
                LaunchButton.Text = "Not Logged Into Steam";
            }
            else if (RunningProcessHandle != IntPtr.Zero)
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
                    Config.ManualImport ? "Not Advertised" : Config.PlayerCount.ToString(),
                    Config.Description
                }, Config.PasswordRequired ? 0 : -1);
                ImportedServerListView.Items.Add(Item);
            }
        }

        private void OnLoaded(object sender, EventArgs e)
        {
            string PredictedInstallPath = SteamUtils.GetGameInstallPath("DARK SOULS III") + @"Game\DarkSoulsIII.exe";
            if (!File.Exists(ProgramSettings.Default.exe_location) && File.Exists(PredictedInstallPath))
            {
                ProgramSettings.Default.exe_location = PredictedInstallPath;
            }

            ExeLocationTextBox.Text = ProgramSettings.Default.exe_location;
            ServerConfigList.FromJson(ProgramSettings.Default.server_config_json, out ServerList);

            // Strip out any old config files downloaded from the server, we will be querying them
            // shortly anyway.
            foreach (ServerConfig Config in ServerList.Servers.ToArray())
            {
                if (!Config.ManualImport)
                {
                    ServerList.Servers.Remove(Config);
                }
            }

            ValidateUI();
            BuildServerList();
            QueryServers();

            ContinualUpdateTimer.Enabled = ShouldRunContinualUpdate();
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

        private void QueryServers()
        {
            Debug.WriteLine("Querying master server ...");

            if (QueryServerTask != null && !QueryServerTask.IsCompleted)
            {
                return;
            }

            QueryServerTask = Task.Run(() =>
            {
                List<ServerConfig> Servers = MasterServerApi.ListServers();
                this.Invoke((MethodInvoker)delegate {
                    ProcessServerQueryResponse(Servers);
                });
            });
        }

        private void ProcessServerQueryResponse(List<ServerConfig> Servers)
        {
            foreach (ServerConfig Server in Servers)
            {
                bool Exists = false;
                foreach (ServerConfig ExistingServer in ServerList.Servers)
                {
                    if (ExistingServer.Hostname == Server.Hostname)
                    {
                        ExistingServer.CopyTransientPropsFrom(Server);
                        Exists = true;
                        break;
                    }
                }

                if (!Exists)
                {
                    ServerList.Servers.Add(Server);
                }
            }

            BuildServerList();
        }

        private void OnLaunch(object sender, EventArgs e)
        {
            ServerConfig Config = ServerList.Servers[ImportedServerListView.SelectedIndices[0]];

            if (Config.PasswordRequired && string.IsNullOrEmpty(Config.PublicKey))
            {
                Forms.PasswordDialog Dialog = new Forms.PasswordDialog(Config);
                if (Dialog.ShowDialog() != DialogResult.OK || string.IsNullOrEmpty(Config.PublicKey))
                {
                    return;
                }
            }
            
            PerformLaunch(Config);
        }

        void PerformLaunch(ServerConfig Config)
        {
            if (Config.PublicKey == null || Config.PublicKey.Length == 0)
            {
                MessageBox.Show("Unable to launch server, no public key is available.\n\nYou shouldn't see this error unless someone has miss-configured the server configuration.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

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
            ContinualUpdateTimer.Enabled = ShouldRunContinualUpdate();

            ValidateUI();
        }

        private bool ShouldRunContinualUpdate()
        {
            /*
            if (RunningProcessHandle != IntPtr.Zero)
            {
                return true;
            }

            if (!SteamUtils.IsSteamRunningAndLoggedIn())
            {
                return true;
            }

            return false;
            */
            return true;
        }

        private void OnContinualUpdateTimer(object sender, EventArgs e)
        {
            uint ExitCode = 0;
            if (RunningProcessHandle != IntPtr.Zero)
            {
                if (!WinAPI.GetExitCodeProcess(RunningProcessHandle, out ExitCode) || ExitCode != (uint)ProcessExitCodes.STILL_ACTIVE)
                {
                    RunningProcessHandle = IntPtr.Zero;
                }
            }

            ValidateUI();

            //ContinualUpdateTimer.Enabled = ShouldRunContinualUpdate();
        }

        private void OnServerRefreshTimer(object sender, EventArgs e)
        {
            QueryServers();
        }
    }
}
