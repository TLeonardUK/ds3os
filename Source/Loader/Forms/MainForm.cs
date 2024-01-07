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
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Window;

namespace Loader
{
    public enum GameType
    {
        DarkSouls3,
        DarkSouls2
    }

    public partial class MainForm : Form
    {
        private ServerConfigList ServerList = new ServerConfigList();
        private IntPtr RunningProcessHandle = IntPtr.Zero;
        private uint RunningProcessId = 0;
        private Task QueryServerTask = null;

        private GameType CurrentGameType = GameType.DarkSouls3;

        private bool IgnoreInputChanges = false;

        private string MachinePrivateIp = "";
        private string MachinePublicIp = "";

        private string[] ColumnNames = { "Server Name", "Player Count", "Description" };
        
        public static string OfficialServer = NetUtils.HostnameToIPv4("ds3os-server.timleonard.uk");

        public MainForm()
        {
            InitializeComponent();

            ImportedServerListView.Items.Clear();
            ImportedServerListView.ListViewItemSorter = new ServerListSorter();

            MachinePrivateIp = NetUtils.GetMachineIPv4(false);
            MachinePublicIp = NetUtils.GetMachineIPv4(true);
        }

        private void SaveConfig()
        {        
            switch (CurrentGameType)
            {
                case GameType.DarkSouls3:
                {
                    ProgramSettings.Default.ds3_exe_location = ExeLocationTextBox.Text;
                    break;
                }
                case GameType.DarkSouls2:
                {
                    ProgramSettings.Default.ds2_exe_location = ExeLocationTextBox.Text;
                    break;
                }
            }

            ProgramSettings.Default.server_config_json = ServerList.ToJson();
            ProgramSettings.Default.hide_passworded = hidePasswordedBox.Checked;
            ProgramSettings.Default.minimum_players = (int)minimumPlayersBox.Value;

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

            bool HasSelectedManualServer = false;
            if (ImportedServerListView.SelectedIndices.Count > 0)
            {
                HasSelectedManualServer = GetConfigFromId((ImportedServerListView.SelectedItems[0].Tag as ServerConfig).Id).ManualImport;
            }
            //RemoveButton.Enabled = HasSelectedManualServer;

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
            
            if (Config.GameType != CurrentGameType.ToString())
            {
                return false;
            }

            string filter = filterBox.Text.ToLower();
            if (!string.IsNullOrWhiteSpace(filter))
            {
                if (!Config.Name.ToLower().Contains(filter) && !Config.Description.ToLower().Contains(filter))
                {
                    return false;
                }
            }
            else
            {
                if (Config.PasswordRequired && hidePasswordedBox.Checked)
                {
                    return false;
                }
                if (Config.PlayerCount < minimumPlayersBox.Value)
                {
                    return false;
                }
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
                    if ((ViewItem.Tag as ServerConfig).Id == Config.Id)
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

                bool IsOfficial = (Config.Hostname == OfficialServer && !Config.IsShard);

                ServerItem.Text = Config.Name;
                ServerItem.Tag = Config;
                ServerItem.SubItems[0].Text = Config.Name;
                ServerItem.SubItems[1].Text = Config.ManualImport ? "Not Available For Manual Import" : Config.PlayerCount.ToString();
                ServerItem.SubItems[2].Text = Config.Description;
                ServerItem.BackColor = (IsOfficial ? Color.PaleGoldenrod : Color.Transparent);

                if (IsOfficial)
                {
                    ServerItem.ImageIndex = 10;
                }
                else if (Config.PasswordRequired)
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
                    if (Config.Id == (ViewItem.Tag as ServerConfig).Id)
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
            if (!File.Exists(ProgramSettings.Default.ds3_exe_location) && File.Exists(PredictedInstallPath))
            {
                ProgramSettings.Default.ds3_exe_location = PredictedInstallPath;
            }
            
            PredictedInstallPath = SteamUtils.GetGameInstallPath("Dark Souls II Scholar of the First Sin") + @"\Game\DarkSoulsII.exe";
            if (!File.Exists(ProgramSettings.Default.ds2_exe_location) && File.Exists(PredictedInstallPath))
            {
                ProgramSettings.Default.ds2_exe_location = PredictedInstallPath;
            }

            IgnoreInputChanges = true;
            gameTabControl.SelectedIndex = 1;
            ExeLocationTextBox.Text = ProgramSettings.Default.ds3_exe_location;
            hidePasswordedBox.Checked = ProgramSettings.Default.hide_passworded;
            minimumPlayersBox.Value = ProgramSettings.Default.minimum_players;
            ServerConfigList.FromJson(ProgramSettings.Default.server_config_json, out ServerList);
            IgnoreInputChanges = false;
            
#if false//DEBUG
            ProgramSettings.Default.Reset();
            ProgramSettings.Default.master_server_url = "http://127.0.0.1:50020";
#endif
            
            // Strip out any old config files downloaded from the server, we will be querying them
            // shortly anyway.
            foreach (ServerConfig Config in ServerList.Servers.ToArray())
            {
                if (!Config.ManualImport)
                {
                    ServerList.Servers.Remove(Config);
                }
            }
            
            ApplyTabSettings();

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
                switch (CurrentGameType)
                {
                    case GameType.DarkSouls3:
                    {
                        Dialog.Filter = "Dark Souls III|DarkSoulsIII.exe|All Files|*.*";
                        Dialog.Title = "Select DS3 Executable Location";
                        break;
                    }
                    case GameType.DarkSouls2:
                    {
                        Dialog.Filter = "Dark Souls II|DarkSoulsII.exe|All Files|*.*";
                        Dialog.Title = "Select DS2 Executable Location";
                        break;
                    }
                }

                if (Dialog.ShowDialog() == DialogResult.OK)
                {
                    ExeLocationTextBox.Text = Dialog.FileName;

                    SaveConfig();
                    ValidateUI();
                }
            }
        }

        private void OnCreateNewServer(object sender, EventArgs e)
        {        
            Forms.CreateServerDialog Dialog = new Forms.CreateServerDialog(ServerList.Servers, MachinePublicIp, this, CurrentGameType);
            if (Dialog.ShowDialog() != DialogResult.OK)
            {
                return;
            }
        }

        private ServerConfig CurrentServerConfig;

        private void OnSelectedServerChanged(object sender, EventArgs e)
        {
            if (ImportedServerListView.SelectedItems.Count > 0)
            {
                CurrentServerConfig = GetConfigFromId((ImportedServerListView.SelectedItems[0].Tag as ServerConfig).Id);
            }

            ValidateUI();
            UpdateServerIp();
        }

        private void OnRemoveClicked(object sender, EventArgs e)
        {
            if (ImportedServerListView.SelectedItems.Count > 0)
            {
                ServerConfig Config = GetConfigFromId((ImportedServerListView.SelectedItems[0].Tag as ServerConfig).Id);

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
                    if (ExistingServer.Id == Server.Id)
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

                    if (Server1.Id == Server2.Id)
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
                    if (ExistingServer.Id == Server.Id)
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

        private ServerConfig GetConfigFromId(string Id)
        {
            for (int i = 0; i < ServerList.Servers.Count; i++)
            {
                if (ServerList.Servers[i].Id == Id)
                {
                    return ServerList.Servers[i];
                }
            }

            return null;
        }

        private void OnLaunch(object sender, EventArgs e)
        {
            ServerConfig Config = GetConfigFromId((ImportedServerListView.SelectedItems[0].Tag as ServerConfig).Id);

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
                        Config.PublicKey = MasterServerApi.GetPublicKey(Config.Id, "");
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
            
            DarkSoulsLoadConfig LoadConfig;
            if (!BuildConfig.ExeLoadConfiguration.TryGetValue(ExeUtils.GetExeSimpleHash(ExeLocationTextBox.Text), out LoadConfig))
            {
                MessageBox.Show("Failed to determine exe version, unable to patch.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string ExeLocation = ExeLocationTextBox.Text;
            string ExeDirectory = Path.GetDirectoryName(ExeLocation);
 
            string AppIdFile = Path.Combine(ExeDirectory, "steam_appid.txt");
            File.WriteAllText(AppIdFile, LoadConfig.SteamAppId.ToString());

            STARTUPINFO StartupInfo = new STARTUPINFO();
            PROCESS_INFORMATION ProcessInfo = new PROCESS_INFORMATION();

            bool Result = WinAPI.CreateProcess(
                null,
                "\"" + ExeLocation + "\"",
                IntPtr.Zero,
                IntPtr.Zero,
                false,
                ProcessCreationFlags.ZERO_FLAG, 
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

            // Inject our hook DLL.
            if (LoadConfig.UseInjector)
            {
                // Find injector DLL.
                string DirectoryPath = System.IO.Path.GetDirectoryName(Application.ExecutablePath);
                string InjectorPath = System.IO.Path.Combine(DirectoryPath, "Injector.dll");
                string InjectorConfigPath = System.IO.Path.Combine(DirectoryPath, "Injector.config");
                while (!File.Exists(InjectorPath))
                {
                    DirectoryPath = System.IO.Path.GetDirectoryName(DirectoryPath);
                    if (DirectoryPath == null)
                    {
                        MessageBox.Show("Failed to find Injector.dll, please reinstall: GetLastError=" + Marshal.GetLastWin32Error(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }

                    InjectorPath = System.IO.Path.Combine(DirectoryPath, "Injector.dll");
                    InjectorConfigPath = System.IO.Path.Combine(DirectoryPath, "Injector.config");
                }

                byte[] InjectorPathBuffer = System.Text.Encoding.Unicode.GetBytes(InjectorPath + "\0");

                // Write the config file which the injector will read everything from.
                InjectionConfig injectConfig = new InjectionConfig();
                injectConfig.ServerName = Config.Name;
                injectConfig.ServerPublicKey = Config.PublicKey;
                injectConfig.ServerHostname = ConnectionHostname;
                injectConfig.ServerPort = Config.Port;
                injectConfig.ServerGameType = Config.GameType;
                injectConfig.EnableSeperateSaveFiles = ProgramSettings.Default.use_seperate_saves;

                string json = injectConfig.ToJson();
                File.WriteAllText(InjectorConfigPath, json);

                // Inject the DLL into the process.
                IntPtr ModulePtr = WinAPI.GetModuleHandle("kernel32.dll");
                if (ModulePtr == IntPtr.Zero)
                {
                    MessageBox.Show("Failed to get kernel32.dll module handle: GetLastError=" + Marshal.GetLastWin32Error(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                IntPtr LoadLibraryPtr = WinAPI.GetProcAddress(ModulePtr, "LoadLibraryW");
                if (LoadLibraryPtr == IntPtr.Zero)
                {
                    MessageBox.Show("Failed to get LoadLibraryA procedure address: GetLastError=" + Marshal.GetLastWin32Error(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                IntPtr PathAddress = IntPtr.Zero;
                for (int i = 0; i < 32 && PathAddress == IntPtr.Zero; i++)
                {
                    PathAddress = WinAPI.VirtualAllocEx(ProcessInfo.hProcess, IntPtr.Zero, (uint)InjectorPathBuffer.Length, (uint)(AllocationType.Reserve | AllocationType.Commit), (uint)MemoryProtection.ReadWrite);
                    if (PathAddress == IntPtr.Zero)
                    {
                        Thread.Sleep(500);
                    }
                }
                if (PathAddress == IntPtr.Zero)
                {
                    MessageBox.Show("Failed to allocation memory in process: GetLastError=" + Marshal.GetLastWin32Error(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);                    
                    return;
                }
                
                int BytesWritten;
                bool WriteSuccessful = WinAPI.WriteProcessMemory(ProcessInfo.hProcess, PathAddress, InjectorPathBuffer, (uint)InjectorPathBuffer.Length, out BytesWritten);
                if (!WriteSuccessful || BytesWritten != InjectorPathBuffer.Length)
                {
                    MessageBox.Show("Failed to write full patch to memory: GetLastError=" + Marshal.GetLastWin32Error(), "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }

                IntPtr ThreadHandle = WinAPI.CreateRemoteThread(ProcessInfo.hProcess, IntPtr.Zero, 0, LoadLibraryPtr, PathAddress, 0, IntPtr.Zero);
                if (ThreadHandle == IntPtr.Zero)
                {
                    MessageBox.Show("Failed to spawn remote thread: GetLastError=" + Marshal.GetLastWin32Error(), "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
            }

            // Otherwise patch the server key into the process memory.
            else
            {            
                byte[] DataBlock = PatchingUtils.MakeEncryptedServerInfo(ConnectionHostname, Config.PublicKey, LoadConfig.Key);
                if (DataBlock == null)
                {
                    MessageBox.Show("Failed to encode server info patch. Potentially server information is too long to fit into the space available.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // Retry a few times until steamstub has unpacked everything.
                // Ugly as fuck, but simplest way to handle this.
                for (int i = 0; i < 32; i++)
                {
                    IntPtr BaseAddress = WinAPI.GetProcessModuleBaseAddress(ProcessInfo.hProcess);
                    IntPtr PatchAddress = (IntPtr)LoadConfig.ServerInfoAddress;
                    if (LoadConfig.UsesASLR)
                    {
                        PatchAddress = (IntPtr)((ulong)BaseAddress + (ulong)PatchAddress);
                    }

                    int BytesWritten;
                    bool WriteSuccessful = WinAPI.WriteProcessMemory(ProcessInfo.hProcess, PatchAddress, DataBlock, (uint)DataBlock.Length, out BytesWritten);
                    if (!WriteSuccessful || BytesWritten != DataBlock.Length)
                    {
                        if (i == 31)
                        {
                            MessageBox.Show("Failed to write full patch to memory. Game may or may not work.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        }
                        else
                        {
                            Thread.Sleep(500);
                        }
                    }            
                    else
                    {
                        break;
                    }
                }
            }

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
                WinAPIProcesses.KillMutex(ExistingProcess, "\\BaseNamedObjects\\DarkSoulsIIMutex");
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

        private void OnClickGithubLink(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(new ProcessStartInfo
            {
                FileName = "https://github.com/tleonarduk/ds3os",
                UseShellExecute = true
            });
        }

        private void OnFilterPropertyChanged(object sender, EventArgs e)
        {
            if (IgnoreInputChanges)
            {
                return;
            }

            SaveConfig();
            BuildServerList();
        }

        private void OnClickDiscordLink(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(new ProcessStartInfo
            {
                FileName = "https://discord.gg/pBmquc9Jkj",
                UseShellExecute = true
            });
        }

        private void OnColumnClicked(object sender, ColumnClickEventArgs e)
        {
            ServerListSorter Sorter = ImportedServerListView.ListViewItemSorter as ServerListSorter;
            if (Sorter.SortColumn != e.Column)
            {
                if (Sorter.SortColumn != -1)
                {
                    ImportedServerListView.Columns[Sorter.SortColumn].Text = ColumnNames[Sorter.SortColumn];
                }
                Sorter.SortColumn = e.Column;
            }
            
            Sorter.SortOrder = (Sorter.SortOrder + 1) % 3;

            if (Sorter.SortOrder == 0)
            {
                ImportedServerListView.Columns[Sorter.SortColumn].Text = ColumnNames[Sorter.SortColumn];
            }
            else if (Sorter.SortOrder == 1)
            {
                ImportedServerListView.Columns[Sorter.SortColumn].Text = "↑ " + ColumnNames[Sorter.SortColumn];
            }
            else
            {
                ImportedServerListView.Columns[Sorter.SortColumn].Text = "↓ " + ColumnNames[Sorter.SortColumn];
            }
            ImportedServerListView.Sort();
        }

        private void SettingsButton_Click(object sender, EventArgs e)
        {
            SettingsForm dialog = new SettingsForm();
            dialog.ExeLocation = ExeLocationTextBox.Text;
            dialog.ShowDialog();
        }

        private void GameTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            ApplyTabSettings();
            ValidateUI();
            BuildServerList();
        }

        private void ApplyTabSettings()
        {
            CurrentGameType = GameType.DarkSouls2;
            if (gameTabControl.SelectedIndex == 1)
            {
                CurrentGameType = GameType.DarkSouls3;
            }

            switch (CurrentGameType)
            {
                case GameType.DarkSouls3:
                {
                    ExeLocationTextBox.Text = ProgramSettings.Default.ds3_exe_location;
                    ExePathLabel.Text = "DarkSoulsIII.exe Location";
                    break;
                }
                case GameType.DarkSouls2:
                {
                    ExeLocationTextBox.Text = ProgramSettings.Default.ds2_exe_location;
                    ExePathLabel.Text = "DarkSoulsII.exe Location";
                    break;
                }
            }
        }
    }

    class ServerListSorter : System.Collections.IComparer
    {
        public int SortColumn = 1;
        public int SortOrder = 0; // 0="Smart" Order, 1=Ascending, 2=Descending

        public int Compare(object x, object y)
        {
            ServerConfig a = (x as ListViewItem).Tag as ServerConfig;
            ServerConfig b = (y as ListViewItem).Tag as ServerConfig;

            if (a == null || b == null)
            {
                return 0;
            }

            if (SortOrder == 0)
            {
                // Official server is always first.
                if (a.Hostname == MainForm.OfficialServer)
                {
                    return -1;
                }
                if (b.Hostname == MainForm.OfficialServer)
                {
                    return 1;
                }

                // Imported comes before public which comes before private.
                if (a.ManualImport != b.ManualImport)
                {
                    return (b.ManualImport ? 1 : 0) - (a.ManualImport ? 1 : 0);
                }
                if (a.PasswordRequired != b.PasswordRequired)
                {
                    return (b.ManualImport ? 1 : 0) - (a.ManualImport ? 1 : 0);
                }

                // Sort in each group by player count.
                return b.PlayerCount - a.PlayerCount;
            }
            else
            {
                int Result = 0;
                if (SortColumn == 0)
                {
                    Result = a.Name.CompareTo(b.Name);
                }
                else if (SortColumn == 1)
                {
                    Result = b.PlayerCount - a.PlayerCount;
                }
                else
                {
                    Result = a.Description.CompareTo(b.Description);
                }

                if (SortOrder == 2)
                {
                    Result = -Result;
                }

                return Result;
            }
        }
    }
}
