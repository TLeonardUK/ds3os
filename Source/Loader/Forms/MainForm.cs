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
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace Loader
{
    public partial class MainForm : Form
    {
        private ServerConfigList ServerList = new ServerConfigList();
        private IntPtr RunningProcessHandle = IntPtr.Zero;
        private uint RunningProcessId = 0;
        private Task QueryServerTask = null;

        private string MachinePrivateIp = "";
        private string MachinePublicIp = "";
        private int columnSortLastClickedItem = 1;
        private bool columnSortRowsAscending = true;

        public MainForm()
        {
            InitializeComponent();

            ImportedServerListView.ListViewItemSorter = new ServerListSorter(1, true);
            ImportedServerListView.ColumnClick += new ColumnClickEventHandler(handleColumnClick);

            MachinePrivateIp = NetUtils.GetMachineIPv4(false);
            MachinePublicIp = NetUtils.GetMachineIPv4(true);
        }

        private void SaveConfig()
        {
            ProgramSettings.Default.exe_location = ExeLocationTextBox.Text;
            ProgramSettings.Default.server_config_json = ServerList.ToJson();
            ProgramSettings.Default.hide_passworded = hidePasswordedBox.Checked;
            ProgramSettings.Default.minimum_players = (int)minimumPlayersBox.Value;

            ProgramSettings.Default.Save();
        }

        public void handleColumnClick(object o, ColumnClickEventArgs e)
        {
            bool newAscending = e.Column == columnSortLastClickedItem
                ? (columnSortRowsAscending ? false : true)
                : true;
            columnSortLastClickedItem = e.Column;
            columnSortRowsAscending = newAscending;
            ImportedServerListView.ListViewItemSorter = new ServerListSorter(e.Column, newAscending);
            ImportedServerListView.Sort();
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

            bool HasSelectedManualServer = false;
            if (ImportedServerListView.SelectedIndices.Count > 0)
            {
                HasSelectedManualServer = GetConfigFromHostname(ImportedServerListView.SelectedItems[0].Tag as string).ManualImport;
            }
            RemoveButton.Enabled = HasSelectedManualServer;

            if (ImportedServerListView.SelectedItems.Count <= 0)
            {
                LaunchEnabled = false;
            }

            if (!SteamUtils.IsSteamRunningAndLoggedIn())
            {
                LaunchEnabled = false;
                LaunchButton.Text = "Not Logged Into Steam";
            } 
#if RELEASE
            else if (RunningProcessHandle != IntPtr.Zero)
            {
                LaunchEnabled = false;
                LaunchButton.Text = "Running ...";
            }
#endif
            else
            {
                LaunchButton.Text = "Launch Game";
            }

            LaunchButton.Enabled = LaunchEnabled;

            RefreshButton.Enabled = (QueryServerTask != null);
        }

        private bool ShouldShowServer(ServerConfig Config)
        {
            if (Config.ManualImport)
            {
                return true;
            }
            if (Config.PasswordRequired && ProgramSettings.Default.hide_passworded)
            {
                return false;
            }
            if (Config.PlayerCount < ProgramSettings.Default.minimum_players)
            {
                return false;
            }
            return true;
        }

        private void BuildServerList()
        {
            foreach (ServerConfig Config in ServerList.Servers)
            {
                if (!ShouldShowServer(Config))
                {
                    continue;
                }

                ListViewItem ServerItem = null;

                foreach (ListViewItem ViewItem in ImportedServerListView.Items)
                {
                    if ((string)ViewItem.Tag == Config.Hostname)
                    {
                        ServerItem = ViewItem;
                        break;
                    }
                }

                if (ServerItem == null)
                {
                    ServerItem = new ListViewItem(new string[3], -1);
                    ImportedServerListView.Items.Add(ServerItem);
                }

                ServerItem.Text = Config.Name;
                ServerItem.Tag = Config.Hostname;
                ServerItem.SubItems[0].Text = Config.Name;
                ServerItem.SubItems[1].Text = Config.ManualImport ? "Not Advertised" : Config.PlayerCount.ToString();
                ServerItem.SubItems[2].Text = Config.Description;

                if (Config.PasswordRequired)
                {
                    ServerItem.ImageIndex = 0;
                }
                else if (Config.ManualImport)
                {
                    ServerItem.ImageIndex = 7;
                }
                else
                {
                    ServerItem.ImageIndex = 8;
                }
                
            }

            for (int i = 0; i < ImportedServerListView.Items.Count; /* empty */)
            {
                ListViewItem ViewItem = ImportedServerListView.Items[i];

                bool Exists = false;

                foreach (ServerConfig Config in ServerList.Servers)
                {
                    if (!ShouldShowServer(Config))
                    {
                        continue;
                    }
                    if (Config.Hostname == (string)ViewItem.Tag)
                    {
                        Exists = true;
                        break;
                    }
                }

                if (!Exists)
                {
                    ImportedServerListView.Items.RemoveAt(i);
                }
                else
                {
                    i++;
                }
            }

            ImportedServerListView.Sort();
        }

        private void OnLoaded(object sender, EventArgs e)
        {
            string PredictedInstallPath = SteamUtils.GetGameInstallPath("DARK SOULS III") + @"\Game\DarkSoulsIII.exe";
            if (!File.Exists(ProgramSettings.Default.exe_location) && File.Exists(PredictedInstallPath))
            {
                ProgramSettings.Default.exe_location = PredictedInstallPath;
            }

            ExeLocationTextBox.Text = ProgramSettings.Default.exe_location;
            hidePasswordedBox.Checked = ProgramSettings.Default.hide_passworded;
            minimumPlayersBox.Value = ProgramSettings.Default.minimum_players;
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

            privateIpBox.Text = MachinePrivateIp;
            publicIpBox.Text = MachinePublicIp;
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

        private ServerConfig CurrentServerConfig;

        private void OnSelectedServerChanged(object sender, EventArgs e)
        {
            if (ImportedServerListView.SelectedItems.Count > 0)
            {
                CurrentServerConfig = GetConfigFromHostname(ImportedServerListView.SelectedItems[0].Tag as string);
            }

            ValidateUI();
            UpdateServerIp();
        }

        private void OnRemoveClicked(object sender, EventArgs e)
        {
            if (ImportedServerListView.SelectedItems.Count > 0)
            {
                ServerConfig Config = GetConfigFromHostname(ImportedServerListView.SelectedItems[0].Tag as string);

                for (int i = 0; i < ServerList.Servers.Count; i++)
                {
                    if (ServerList.Servers[i].Hostname == Config.Hostname)
                    {
                        ServerList.Servers.RemoveAt(i);
                        break;
                    }
                }

                BuildServerList();
                SaveConfig();
                ValidateUI();
            }
        }

        private void UpdateServerIp()
        {
            ServerConfig UpdateConfig = CurrentServerConfig;

            QueryServerTask = Task.Run(() =>
            {
                string Ip = ResolveConnectIp(UpdateConfig);

                this.Invoke((MethodInvoker)delegate
                {
                    if (CurrentServerConfig != null && CurrentServerConfig.Hostname == UpdateConfig.Hostname)
                    {
                        serverIpBox.Text = Ip;
                    }
                });
            });
        }

        private void QueryServers()
        {
            Debug.WriteLine("Querying master server ...");

            if (QueryServerTask != null && !QueryServerTask.IsCompleted)
            {
                return;
            }

            RefreshButton.Enabled = false;

            QueryServerTask = Task.Run(() =>
            {
                List<ServerConfig> Servers = MasterServerApi.ListServers();
                if (Servers != null)
                {
                    this.Invoke((MethodInvoker)delegate
                    {
                        ProcessServerQueryResponse(Servers);
                    });
                }
                this.Invoke((MethodInvoker)delegate
                {
                    RefreshButton.Enabled = true;
                });
            });
        }

        private void ProcessServerQueryResponse(List<ServerConfig> Servers)
        {
            // Add new entries.
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

            // Remove duplicates
            for (int i = 0; i < ServerList.Servers.Count; /* empty */)
            {
                ServerConfig Server1 = ServerList.Servers[i];

                bool Duplicate = false;
                for (int j = 0; j < ServerList.Servers.Count; j++)
                {
                    ServerConfig Server2 = ServerList.Servers[j];

                    if (i == j)
                    {
                        continue;
                    }

                    if (Server1.Hostname == Server2.Hostname)
                    {
                        Duplicate = true;
                        break;
                    }
                }

                if (Duplicate)
                {
                    ServerList.Servers.RemoveAt(i);
                }
                else
                {
                    i++;
                }
            }

            // Remove servers that no longer exist.
            for (int i = 0; i < ServerList.Servers.Count; /* empty */)
            {
                ServerConfig ExistingServer = ServerList.Servers[i];
                if (ExistingServer.ManualImport)
                {
                    i++;
                    continue;
                }

                bool Exists = false;
                foreach (ServerConfig Server in Servers)
                {
                    if (ExistingServer.Hostname == Server.Hostname)
                    {
                        Exists = true;
                        break;
                    }
                }

                if (!Exists)
                {
                    ServerList.Servers.RemoveAt(i);
                }
                else
                {
                    i++;
                }
            }

            BuildServerList();
        }

        private ServerConfig GetConfigFromHostname(string Hostname)
        {
            for (int i = 0; i < ServerList.Servers.Count; i++)
            {
                if (ServerList.Servers[i].Hostname == Hostname)
                {
                    return ServerList.Servers[i];
                }
            }

            return null;
        }

        private void OnLaunch(object sender, EventArgs e)
        {
            ServerConfig Config = GetConfigFromHostname(ImportedServerListView.SelectedItems[0].Tag as string);

            if (string.IsNullOrEmpty(Config.PublicKey))
            {
                if (Config.PasswordRequired)
                {
                    Forms.PasswordDialog Dialog = new Forms.PasswordDialog(Config);
                    if (Dialog.ShowDialog() != DialogResult.OK || string.IsNullOrEmpty(Config.PublicKey))
                    {
                        return;
                    }
                }
                else
                {
                    Task GetKeyTask = Task.Run(() =>
                    {
                        Config.PublicKey = MasterServerApi.GetPublicKey(Config.IpAddress, "");
                    });

                    while (!GetKeyTask.IsCompleted)
                    {
                        Application.DoEvents();
                    }

                    if (string.IsNullOrEmpty(Config.PublicKey))
                    {
                        MessageBox.Show("Failed to retrieve the servers cryptographic keys.\n\nThe master server may be down or the server may be missconfigured.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }
                }
            }
            
            PerformLaunch(Config);
        }

        string ResolveConnectIp(ServerConfig Config)
        {
            string ConnectionHostname = Config.Hostname;
            string HostnameIp = NetUtils.HostnameToIPv4(Config.Hostname);
            string PrivateHostnameIp = NetUtils.HostnameToIPv4(Config.PrivateHostname);

            // If the servers public ip is the same as the machines public ip, then we are behind
            // the same nat and should use the private hostname instead. 
            //
            // Note: This potentially breaks down with carrier grade NAT.
            // ... We're sort of ignoring that right now as this helps the majority of users.
            // Those behind CGN can manually set the ip's on the servers config to get around this.
            if (HostnameIp == MachinePublicIp)
            {
                // If ip of private hostname and private ip of machine are the same
                // then server is running on same host so just use loopback address.
                if (PrivateHostnameIp == MachinePrivateIp)
                {
                    ConnectionHostname = "127.0.0.1";
                }
                // Otherwise just use the private address.
                else
                {
                    ConnectionHostname = Config.PrivateHostname;
                }
            }

            return ConnectionHostname;
        }

        void PerformLaunch(ServerConfig Config)
        {
            if (Config.PublicKey == null || Config.PublicKey.Length == 0)
            {
                MessageBox.Show("Unable to launch server, no public key is available.\n\nYou shouldn't see this error unless someone has miss-configured the server configuration.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            // Try and kill existing process mutex if we can.
            if (RunningProcessHandle != IntPtr.Zero)
            {
#if DEBUG
                KillNamedMutexIfExists();
#endif
            }

            string ConnectionHostname = ResolveConnectIp(Config);

            byte[] DataBlock = PatchingUtils.MakeEncryptedServerInfo(ConnectionHostname, Config.PublicKey);
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
            RunningProcessId = ProcessInfo.dwProcessId;
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

        // Kills the named mutex that dark souls 3 uses to only open a single instance.
        private void KillNamedMutexIfExists()
        {
            Process ExistingProcess = Process.GetProcessById((int)RunningProcessId);
            if (ExistingProcess != null)
            {
                WinAPIProcesses.KillMutex(ExistingProcess, "\\BaseNamedObjects\\DarkSoulsIIIMutex");
            }
        }

        private void OnContinualUpdateTimer(object sender, EventArgs e)
        {
            uint ExitCode = 0;
            if (RunningProcessHandle != IntPtr.Zero)
            {
                // Check if process has finished.
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

        private void OnRefreshClicked(object sender, EventArgs e)
        {
            QueryServers();
        }

        private void ClickGithubLink(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(new ProcessStartInfo
            {
                FileName = "https://github.com/tleonarduk/ds3os",
                UseShellExecute = true
            });
        }

        private void FilterPropertyChanged(object sender, EventArgs e)
        {
            SaveConfig();
            BuildServerList();
        }

        private void ClickDiscordLink(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(new ProcessStartInfo
            {
                FileName = "https://discord.gg/AVaNS26v",
                UseShellExecute = true
            });
        }
    }

    class ServerListSorter : System.Collections.IComparer
    {
        public int column = 1;
        public bool asc = true;

        public ServerListSorter(int column, bool asc)
        {
            this.column = column;
            this.asc = asc;
        }

        public int Compare(object x, object y)
        {
            ListViewItem a = (ListViewItem)x;
            ListViewItem b = (ListViewItem)y;

            if (a.ImageIndex == b.ImageIndex)
            {
                int aPlayerCount = 0;
                int bPlayerCount = 0;
                if (int.TryParse(a.SubItems[this.column].Text, out aPlayerCount)
                    && int.TryParse(b.SubItems[this.column].Text, out bPlayerCount))

                {
                    return asc
                        ? bPlayerCount - aPlayerCount
                        : aPlayerCount - bPlayerCount;
                }
                else
                {
                    return asc
                        ? string.Compare(a.SubItems[this.column].Text, b.SubItems[this.column].Text)
                        : string.Compare(b.SubItems[this.column].Text, a.SubItems[this.column].Text);
                }

            }
            else
            {
                if (a.ImageIndex == 7)
                {
                    return -1;
                }
                else if (b.ImageIndex == 7)
                {
                    return 1;
                }
                else
                {
                    return b.ImageIndex - a.ImageIndex;
                }
            }
        }
    }
}
