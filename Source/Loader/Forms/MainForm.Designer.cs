/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

namespace Loader
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
        components = new System.ComponentModel.Container();
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
        System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] { "Name", "Players", "Description" }, 0);
        ImportButton = new System.Windows.Forms.Button();
        serverListImageList = new System.Windows.Forms.ImageList(components);
        LaunchButton = new System.Windows.Forms.Button();
        ExePathLabel = new System.Windows.Forms.Label();
        ExeLocationTextBox = new System.Windows.Forms.TextBox();
        ExeLocationBrowseButton = new System.Windows.Forms.Button();
        ImportedServerListView = new System.Windows.Forms.ListView();
        columnHeader1 = new System.Windows.Forms.ColumnHeader();
        columnHeader4 = new System.Windows.Forms.ColumnHeader();
        columnHeader2 = new System.Windows.Forms.ColumnHeader();
        BuildInfoLabel = new System.Windows.Forms.Label();
        ContinualUpdateTimer = new System.Windows.Forms.Timer(components);
        panel1 = new System.Windows.Forms.Panel();
        DiscordLink = new System.Windows.Forms.LinkLabel();
        GithubLink = new System.Windows.Forms.LinkLabel();
        serverListRefreshTimer = new System.Windows.Forms.Timer(components);
        RefreshButton = new System.Windows.Forms.Button();
        minimumPlayersBox = new System.Windows.Forms.NumericUpDown();
        label1 = new System.Windows.Forms.Label();
        hidePasswordedBox = new System.Windows.Forms.CheckBox();
        label3 = new System.Windows.Forms.Label();
        label4 = new System.Windows.Forms.Label();
        label5 = new System.Windows.Forms.Label();
        privateIpBox = new System.Windows.Forms.Label();
        publicIpBox = new System.Windows.Forms.Label();
        serverIpBox = new System.Windows.Forms.Label();
        filterBox = new System.Windows.Forms.TextBox();
        panel2 = new System.Windows.Forms.Panel();
        SettingsButton = new System.Windows.Forms.Button();
        gameTabControl = new System.Windows.Forms.TabControl();
        tabPage1 = new System.Windows.Forms.TabPage();
        tabPage2 = new System.Windows.Forms.TabPage();
        panel3 = new System.Windows.Forms.Panel();
        panel1.SuspendLayout();
        ((System.ComponentModel.ISupportInitialize)minimumPlayersBox).BeginInit();
        gameTabControl.SuspendLayout();
        SuspendLayout();
        // 
        // ImportButton
        // 
        ImportButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
        ImportButton.ImageKey = "add.png";
        ImportButton.ImageList = serverListImageList;
        ImportButton.Location = new System.Drawing.Point(8, 414);
        ImportButton.Name = "ImportButton";
        ImportButton.Size = new System.Drawing.Size(166, 35);
        ImportButton.TabIndex = 1;
        ImportButton.Text = "Create New Server";
        ImportButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
        ImportButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
        ImportButton.UseVisualStyleBackColor = true;
        ImportButton.Click += OnCreateNewServer;
        // 
        // serverListImageList
        // 
        serverListImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
        serverListImageList.ImageStream = (System.Windows.Forms.ImageListStreamer)resources.GetObject("serverListImageList.ImageStream");
        serverListImageList.TransparentColor = System.Drawing.Color.Transparent;
        serverListImageList.Images.SetKeyName(0, "lock.png");
        serverListImageList.Images.SetKeyName(1, "arrow_refresh.png");
        serverListImageList.Images.SetKeyName(2, "cross.png");
        serverListImageList.Images.SetKeyName(3, "add.png");
        serverListImageList.Images.SetKeyName(4, "joystick.png");
        serverListImageList.Images.SetKeyName(5, "magnifier.png");
        serverListImageList.Images.SetKeyName(6, "bullet_star.png");
        serverListImageList.Images.SetKeyName(7, "star.png");
        serverListImageList.Images.SetKeyName(8, "world.png");
        serverListImageList.Images.SetKeyName(9, "folder_magnify.png");
        serverListImageList.Images.SetKeyName(10, "award_star_gold_2.png");
        serverListImageList.Images.SetKeyName(11, "cog.png");
        // 
        // LaunchButton
        // 
        LaunchButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        LaunchButton.ImageIndex = 4;
        LaunchButton.ImageList = serverListImageList;
        LaunchButton.Location = new System.Drawing.Point(751, 414);
        LaunchButton.Name = "LaunchButton";
        LaunchButton.Size = new System.Drawing.Size(174, 35);
        LaunchButton.TabIndex = 2;
        LaunchButton.Text = "Launch Game";
        LaunchButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
        LaunchButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
        LaunchButton.UseVisualStyleBackColor = true;
        LaunchButton.Click += OnLaunch;
        // 
        // ExePathLabel
        // 
        ExePathLabel.AutoSize = true;
        ExePathLabel.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
        ExePathLabel.Location = new System.Drawing.Point(8, 152);
        ExePathLabel.Name = "ExePathLabel";
        ExePathLabel.Size = new System.Drawing.Size(138, 15);
        ExePathLabel.TabIndex = 4;
        ExePathLabel.Text = "DarkSoulsIII.exe Location";
        // 
        // ExeLocationTextBox
        // 
        ExeLocationTextBox.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        ExeLocationTextBox.Location = new System.Drawing.Point(8, 172);
        ExeLocationTextBox.Name = "ExeLocationTextBox";
        ExeLocationTextBox.ReadOnly = true;
        ExeLocationTextBox.Size = new System.Drawing.Size(868, 23);
        ExeLocationTextBox.TabIndex = 5;
        // 
        // ExeLocationBrowseButton
        // 
        ExeLocationBrowseButton.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
        ExeLocationBrowseButton.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
        ExeLocationBrowseButton.ImageIndex = 9;
        ExeLocationBrowseButton.ImageList = serverListImageList;
        ExeLocationBrowseButton.Location = new System.Drawing.Point(883, 171);
        ExeLocationBrowseButton.Name = "ExeLocationBrowseButton";
        ExeLocationBrowseButton.Size = new System.Drawing.Size(42, 25);
        ExeLocationBrowseButton.TabIndex = 6;
        ExeLocationBrowseButton.UseVisualStyleBackColor = true;
        ExeLocationBrowseButton.Click += OnBrowseForExecutable;
        // 
        // ImportedServerListView
        // 
        ImportedServerListView.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        ImportedServerListView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
        ImportedServerListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader4, columnHeader2 });
        ImportedServerListView.FullRowSelect = true;
        ImportedServerListView.GridLines = true;
        ImportedServerListView.HideSelection = false;
        listViewItem1.StateImageIndex = 0;
        ImportedServerListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { listViewItem1 });
        ImportedServerListView.Location = new System.Drawing.Point(8, 232);
        ImportedServerListView.MultiSelect = false;
        ImportedServerListView.Name = "ImportedServerListView";
        ImportedServerListView.Size = new System.Drawing.Size(917, 176);
        ImportedServerListView.SmallImageList = serverListImageList;
        ImportedServerListView.Sorting = System.Windows.Forms.SortOrder.Descending;
        ImportedServerListView.TabIndex = 7;
        ImportedServerListView.UseCompatibleStateImageBehavior = false;
        ImportedServerListView.View = System.Windows.Forms.View.Details;
        ImportedServerListView.ColumnClick += OnColumnClicked;
        ImportedServerListView.SelectedIndexChanged += OnSelectedServerChanged;
        // 
        // columnHeader1
        // 
        columnHeader1.Text = "Server Name";
        columnHeader1.Width = 250;
        // 
        // columnHeader4
        // 
        columnHeader4.Text = "Player Count";
        columnHeader4.Width = 100;
        // 
        // columnHeader2
        // 
        columnHeader2.Text = "Description";
        columnHeader2.Width = 500;
        // 
        // BuildInfoLabel
        // 
        BuildInfoLabel.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        BuildInfoLabel.Location = new System.Drawing.Point(152, 149);
        BuildInfoLabel.Name = "BuildInfoLabel";
        BuildInfoLabel.Size = new System.Drawing.Size(724, 20);
        BuildInfoLabel.TabIndex = 8;
        BuildInfoLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
        // 
        // ContinualUpdateTimer
        // 
        ContinualUpdateTimer.Interval = 5000;
        ContinualUpdateTimer.Tick += OnContinualUpdateTimer;
        // 
        // panel1
        // 
        panel1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        panel1.BackColor = System.Drawing.Color.Black;
        panel1.BackgroundImage = (System.Drawing.Image)resources.GetObject("panel1.BackgroundImage");
        panel1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
        panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        panel1.Controls.Add(DiscordLink);
        panel1.Controls.Add(GithubLink);
        panel1.Location = new System.Drawing.Point(-2, -2);
        panel1.Margin = new System.Windows.Forms.Padding(0);
        panel1.Name = "panel1";
        panel1.Padding = new System.Windows.Forms.Padding(5);
        panel1.Size = new System.Drawing.Size(943, 113);
        panel1.TabIndex = 10;
        // 
        // DiscordLink
        // 
        DiscordLink.ActiveLinkColor = System.Drawing.Color.White;
        DiscordLink.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
        DiscordLink.AutoSize = true;
        DiscordLink.BackColor = System.Drawing.Color.Transparent;
        DiscordLink.Cursor = System.Windows.Forms.Cursors.Hand;
        DiscordLink.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
        DiscordLink.ForeColor = System.Drawing.Color.White;
        DiscordLink.LinkColor = System.Drawing.Color.White;
        DiscordLink.Location = new System.Drawing.Point(216, 76);
        DiscordLink.Name = "DiscordLink";
        DiscordLink.Size = new System.Drawing.Size(95, 21);
        DiscordLink.TabIndex = 13;
        DiscordLink.TabStop = true;
        DiscordLink.Text = "Join Discord";
        DiscordLink.VisitedLinkColor = System.Drawing.Color.Gray;
        DiscordLink.LinkClicked += OnClickDiscordLink;
        // 
        // GithubLink
        // 
        GithubLink.ActiveLinkColor = System.Drawing.Color.White;
        GithubLink.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
        GithubLink.AutoSize = true;
        GithubLink.BackColor = System.Drawing.Color.Transparent;
        GithubLink.Cursor = System.Windows.Forms.Cursors.Hand;
        GithubLink.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
        GithubLink.ForeColor = System.Drawing.Color.White;
        GithubLink.LinkColor = System.Drawing.Color.White;
        GithubLink.Location = new System.Drawing.Point(117, 76);
        GithubLink.Name = "GithubLink";
        GithubLink.Size = new System.Drawing.Size(93, 21);
        GithubLink.TabIndex = 12;
        GithubLink.TabStop = true;
        GithubLink.Text = "Visit GitHub";
        GithubLink.VisitedLinkColor = System.Drawing.Color.Gray;
        GithubLink.LinkClicked += OnClickGithubLink;
        // 
        // serverListRefreshTimer
        // 
        serverListRefreshTimer.Enabled = true;
        serverListRefreshTimer.Interval = 30000;
        serverListRefreshTimer.Tick += OnServerRefreshTimer;
        // 
        // RefreshButton
        // 
        RefreshButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
        RefreshButton.ImageIndex = 1;
        RefreshButton.ImageList = serverListImageList;
        RefreshButton.Location = new System.Drawing.Point(180, 414);
        RefreshButton.Name = "RefreshButton";
        RefreshButton.Size = new System.Drawing.Size(37, 35);
        RefreshButton.TabIndex = 11;
        RefreshButton.UseVisualStyleBackColor = true;
        RefreshButton.Click += OnRefreshClicked;
        // 
        // minimumPlayersBox
        // 
        minimumPlayersBox.Location = new System.Drawing.Point(131, 203);
        minimumPlayersBox.Name = "minimumPlayersBox";
        minimumPlayersBox.Size = new System.Drawing.Size(53, 23);
        minimumPlayersBox.TabIndex = 12;
        minimumPlayersBox.ValueChanged += OnFilterPropertyChanged;
        // 
        // label1
        // 
        label1.AutoSize = true;
        label1.Location = new System.Drawing.Point(190, 207);
        label1.Name = "label1";
        label1.Size = new System.Drawing.Size(100, 15);
        label1.TabIndex = 13;
        label1.Text = "Minimum Players";
        // 
        // hidePasswordedBox
        // 
        hidePasswordedBox.AutoSize = true;
        hidePasswordedBox.Location = new System.Drawing.Point(8, 206);
        hidePasswordedBox.Name = "hidePasswordedBox";
        hidePasswordedBox.Size = new System.Drawing.Size(117, 19);
        hidePasswordedBox.TabIndex = 14;
        hidePasswordedBox.Text = "Hide Passworded";
        hidePasswordedBox.UseVisualStyleBackColor = true;
        hidePasswordedBox.CheckedChanged += OnFilterPropertyChanged;
        // 
        // label3
        // 
        label3.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        label3.Location = new System.Drawing.Point(259, 424);
        label3.Name = "label3";
        label3.Size = new System.Drawing.Size(61, 23);
        label3.TabIndex = 15;
        label3.Text = "Server IP";
        label3.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // label4
        // 
        label4.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        label4.Location = new System.Drawing.Point(386, 424);
        label4.Name = "label4";
        label4.Size = new System.Drawing.Size(102, 15);
        label4.TabIndex = 17;
        label4.Text = "Private IP";
        label4.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // label5
        // 
        label5.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        label5.Location = new System.Drawing.Point(541, 424);
        label5.Name = "label5";
        label5.Size = new System.Drawing.Size(102, 15);
        label5.TabIndex = 19;
        label5.Text = "Public IP";
        label5.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // privateIpBox
        // 
        privateIpBox.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        privateIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
        privateIpBox.Location = new System.Drawing.Point(494, 424);
        privateIpBox.Name = "privateIpBox";
        privateIpBox.Size = new System.Drawing.Size(99, 19);
        privateIpBox.TabIndex = 21;
        privateIpBox.Text = "255.255.255.255";
        // 
        // publicIpBox
        // 
        publicIpBox.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        publicIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
        publicIpBox.Location = new System.Drawing.Point(649, 424);
        publicIpBox.Name = "publicIpBox";
        publicIpBox.Size = new System.Drawing.Size(99, 19);
        publicIpBox.TabIndex = 22;
        publicIpBox.Text = "255.255.255.255";
        // 
        // serverIpBox
        // 
        serverIpBox.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
        serverIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
        serverIpBox.Location = new System.Drawing.Point(328, 424);
        serverIpBox.Name = "serverIpBox";
        serverIpBox.Size = new System.Drawing.Size(99, 19);
        serverIpBox.TabIndex = 23;
        serverIpBox.Text = "255.255.255.255";
        // 
        // filterBox
        // 
        filterBox.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        filterBox.Location = new System.Drawing.Point(549, 202);
        filterBox.Name = "filterBox";
        filterBox.Size = new System.Drawing.Size(376, 23);
        filterBox.TabIndex = 24;
        filterBox.TextChanged += OnFilterPropertyChanged;
        // 
        // panel2
        // 
        panel2.BackgroundImage = Properties.Resources.magnifier;
        panel2.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
        panel2.Location = new System.Drawing.Point(520, 202);
        panel2.Name = "panel2";
        panel2.Size = new System.Drawing.Size(23, 23);
        panel2.TabIndex = 25;
        // 
        // SettingsButton
        // 
        SettingsButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
        SettingsButton.ImageIndex = 11;
        SettingsButton.ImageList = serverListImageList;
        SettingsButton.Location = new System.Drawing.Point(223, 414);
        SettingsButton.Name = "SettingsButton";
        SettingsButton.Size = new System.Drawing.Size(37, 35);
        SettingsButton.TabIndex = 26;
        SettingsButton.UseVisualStyleBackColor = true;
        SettingsButton.Click += SettingsButton_Click;
        // 
        // gameTabControl
        // 
        gameTabControl.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        gameTabControl.Controls.Add(tabPage1);
        gameTabControl.Controls.Add(tabPage2);
        gameTabControl.Location = new System.Drawing.Point(8, 117);
        gameTabControl.Name = "gameTabControl";
        gameTabControl.SelectedIndex = 0;
        gameTabControl.Size = new System.Drawing.Size(917, 21);
        gameTabControl.TabIndex = 27;
        gameTabControl.SelectedIndexChanged += GameTabControl_SelectedIndexChanged;
        // 
        // tabPage1
        // 
        tabPage1.Location = new System.Drawing.Point(4, 24);
        tabPage1.Name = "tabPage1";
        tabPage1.Padding = new System.Windows.Forms.Padding(3);
        tabPage1.Size = new System.Drawing.Size(909, 0);
        tabPage1.TabIndex = 0;
        tabPage1.Text = "Dark Souls II";
        tabPage1.UseVisualStyleBackColor = true;
        // 
        // tabPage2
        // 
        tabPage2.Location = new System.Drawing.Point(4, 24);
        tabPage2.Name = "tabPage2";
        tabPage2.Padding = new System.Windows.Forms.Padding(3);
        tabPage2.Size = new System.Drawing.Size(909, 0);
        tabPage2.TabIndex = 1;
        tabPage2.Text = "Dark Souls III";
        tabPage2.UseVisualStyleBackColor = true;
        // 
        // panel3
        // 
        panel3.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
        panel3.BackColor = System.Drawing.SystemColors.ControlDark;
        panel3.Location = new System.Drawing.Point(8, 141);
        panel3.Name = "panel3";
        panel3.Size = new System.Drawing.Size(917, 1);
        panel3.TabIndex = 28;
        // 
        // MainForm
        // 
        AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
        AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        ClientSize = new System.Drawing.Size(934, 458);
        Controls.Add(panel3);
        Controls.Add(gameTabControl);
        Controls.Add(SettingsButton);
        Controls.Add(panel2);
        Controls.Add(filterBox);
        Controls.Add(serverIpBox);
        Controls.Add(publicIpBox);
        Controls.Add(privateIpBox);
        Controls.Add(label5);
        Controls.Add(label4);
        Controls.Add(label3);
        Controls.Add(hidePasswordedBox);
        Controls.Add(label1);
        Controls.Add(minimumPlayersBox);
        Controls.Add(RefreshButton);
        Controls.Add(panel1);
        Controls.Add(BuildInfoLabel);
        Controls.Add(ImportedServerListView);
        Controls.Add(ExeLocationBrowseButton);
        Controls.Add(ExeLocationTextBox);
        Controls.Add(ExePathLabel);
        Controls.Add(LaunchButton);
        Controls.Add(ImportButton);
        Icon = (System.Drawing.Icon)resources.GetObject("$this.Icon");
        MinimumSize = new System.Drawing.Size(950, 490);
        Name = "MainForm";
        Text = "Dark Souls - Open Server Loader";
        Load += OnLoaded;
        panel1.ResumeLayout(false);
        panel1.PerformLayout();
        ((System.ComponentModel.ISupportInitialize)minimumPlayersBox).EndInit();
        gameTabControl.ResumeLayout(false);
        ResumeLayout(false);
        PerformLayout();
        }

        #endregion
        private System.Windows.Forms.Button ImportButton;
        private System.Windows.Forms.Button LaunchButton;
        private System.Windows.Forms.Label ExePathLabel;
        private System.Windows.Forms.TextBox ExeLocationTextBox;
        private System.Windows.Forms.Button ExeLocationBrowseButton;
        private System.Windows.Forms.ListView ImportedServerListView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Label BuildInfoLabel;
        private System.Windows.Forms.Timer ContinualUpdateTimer;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.Timer serverListRefreshTimer;
        private System.Windows.Forms.ImageList serverListImageList;
        private System.Windows.Forms.Button RefreshButton;
        private System.Windows.Forms.LinkLabel GithubLink;
        private System.Windows.Forms.NumericUpDown minimumPlayersBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox hidePasswordedBox;
        private System.Windows.Forms.LinkLabel DiscordLink;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label privateIpBox;
        private System.Windows.Forms.Label publicIpBox;
        private System.Windows.Forms.Label serverIpBox;
        private System.Windows.Forms.TextBox filterBox;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Button SettingsButton;
        private System.Windows.Forms.TabControl gameTabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.Panel panel3;
    }
}

