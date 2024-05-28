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
        private System.ComponentModel.ComponentResourceManager resources;
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
            resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
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
            resources.ApplyResources(ImportButton, "ImportButton");
            ImportButton.ImageList = serverListImageList;
            ImportButton.Name = "ImportButton";
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
            resources.ApplyResources(LaunchButton, "LaunchButton");
            LaunchButton.ImageList = serverListImageList;
            LaunchButton.Name = "LaunchButton";
            LaunchButton.UseVisualStyleBackColor = true;
            LaunchButton.Click += OnLaunch;
            // 
            // ExePathLabel
            // 
            resources.ApplyResources(ExePathLabel, "ExePathLabel");
            ExePathLabel.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
            ExePathLabel.Name = "ExePathLabel";
            // 
            // ExeLocationTextBox
            // 
            resources.ApplyResources(ExeLocationTextBox, "ExeLocationTextBox");
            ExeLocationTextBox.Name = "ExeLocationTextBox";
            ExeLocationTextBox.ReadOnly = true;
            // 
            // ExeLocationBrowseButton
            // 
            resources.ApplyResources(ExeLocationBrowseButton, "ExeLocationBrowseButton");
            ExeLocationBrowseButton.ImageList = serverListImageList;
            ExeLocationBrowseButton.Name = "ExeLocationBrowseButton";
            ExeLocationBrowseButton.UseVisualStyleBackColor = true;
            ExeLocationBrowseButton.Click += OnBrowseForExecutable;
            // 
            // ImportedServerListView
            // 
            resources.ApplyResources(ImportedServerListView, "ImportedServerListView");
            ImportedServerListView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            ImportedServerListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader4, columnHeader2 });
            ImportedServerListView.FullRowSelect = true;
            ImportedServerListView.GridLines = true;
            ImportedServerListView.HideSelection = false;
            ImportedServerListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { (System.Windows.Forms.ListViewItem)resources.GetObject("ImportedServerListView.Items") });
            ImportedServerListView.MultiSelect = false;
            ImportedServerListView.Name = "ImportedServerListView";
            ImportedServerListView.SmallImageList = serverListImageList;
            ImportedServerListView.Sorting = System.Windows.Forms.SortOrder.Descending;
            ImportedServerListView.UseCompatibleStateImageBehavior = false;
            ImportedServerListView.View = System.Windows.Forms.View.Details;
            ImportedServerListView.ColumnClick += OnColumnClicked;
            ImportedServerListView.SelectedIndexChanged += OnSelectedServerChanged;
            // 
            // columnHeader1
            // 
            resources.ApplyResources(columnHeader1, "columnHeader1");
            // 
            // columnHeader4
            // 
            resources.ApplyResources(columnHeader4, "columnHeader4");
            // 
            // columnHeader2
            // 
            resources.ApplyResources(columnHeader2, "columnHeader2");
            // 
            // BuildInfoLabel
            // 
            resources.ApplyResources(BuildInfoLabel, "BuildInfoLabel");
            BuildInfoLabel.Name = "BuildInfoLabel";
            // 
            // ContinualUpdateTimer
            // 
            ContinualUpdateTimer.Interval = 5000;
            ContinualUpdateTimer.Tick += OnContinualUpdateTimer;
            // 
            // panel1
            // 
            resources.ApplyResources(panel1, "panel1");
            panel1.BackColor = System.Drawing.Color.Black;
            panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            panel1.Controls.Add(DiscordLink);
            panel1.Controls.Add(GithubLink);
            panel1.Name = "panel1";
            // 
            // DiscordLink
            // 
            resources.ApplyResources(DiscordLink, "DiscordLink");
            DiscordLink.ActiveLinkColor = System.Drawing.Color.White;
            DiscordLink.BackColor = System.Drawing.Color.Transparent;
            DiscordLink.Cursor = System.Windows.Forms.Cursors.Hand;
            DiscordLink.ForeColor = System.Drawing.Color.White;
            DiscordLink.LinkColor = System.Drawing.Color.White;
            DiscordLink.Name = "DiscordLink";
            DiscordLink.TabStop = true;
            DiscordLink.VisitedLinkColor = System.Drawing.Color.Gray;
            DiscordLink.LinkClicked += OnClickDiscordLink;
            // 
            // GithubLink
            // 
            resources.ApplyResources(GithubLink, "GithubLink");
            GithubLink.ActiveLinkColor = System.Drawing.Color.White;
            GithubLink.BackColor = System.Drawing.Color.Transparent;
            GithubLink.Cursor = System.Windows.Forms.Cursors.Hand;
            GithubLink.ForeColor = System.Drawing.Color.White;
            GithubLink.LinkColor = System.Drawing.Color.White;
            GithubLink.Name = "GithubLink";
            GithubLink.TabStop = true;
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
            resources.ApplyResources(RefreshButton, "RefreshButton");
            RefreshButton.ImageList = serverListImageList;
            RefreshButton.Name = "RefreshButton";
            RefreshButton.UseVisualStyleBackColor = true;
            RefreshButton.Click += OnRefreshClicked;
            // 
            // minimumPlayersBox
            // 
            resources.ApplyResources(minimumPlayersBox, "minimumPlayersBox");
            minimumPlayersBox.Name = "minimumPlayersBox";
            minimumPlayersBox.ValueChanged += OnFilterPropertyChanged;
            // 
            // label1
            // 
            resources.ApplyResources(label1, "label1");
            label1.Name = "label1";
            // 
            // hidePasswordedBox
            // 
            resources.ApplyResources(hidePasswordedBox, "hidePasswordedBox");
            hidePasswordedBox.Name = "hidePasswordedBox";
            hidePasswordedBox.UseVisualStyleBackColor = true;
            hidePasswordedBox.CheckedChanged += OnFilterPropertyChanged;
            // 
            // label3
            // 
            resources.ApplyResources(label3, "label3");
            label3.Name = "label3";
            // 
            // label4
            // 
            resources.ApplyResources(label4, "label4");
            label4.Name = "label4";
            // 
            // label5
            // 
            resources.ApplyResources(label5, "label5");
            label5.Name = "label5";
            // 
            // privateIpBox
            // 
            resources.ApplyResources(privateIpBox, "privateIpBox");
            privateIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
            privateIpBox.Name = "privateIpBox";
            // 
            // publicIpBox
            // 
            resources.ApplyResources(publicIpBox, "publicIpBox");
            publicIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
            publicIpBox.Name = "publicIpBox";
            // 
            // serverIpBox
            // 
            resources.ApplyResources(serverIpBox, "serverIpBox");
            serverIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
            serverIpBox.Name = "serverIpBox";
            // 
            // filterBox
            // 
            resources.ApplyResources(filterBox, "filterBox");
            filterBox.Name = "filterBox";
            filterBox.TextChanged += OnFilterPropertyChanged;
            // 
            // panel2
            // 
            resources.ApplyResources(panel2, "panel2");
            panel2.BackgroundImage = Properties.Resources.magnifier;
            panel2.Name = "panel2";
            // 
            // SettingsButton
            // 
            resources.ApplyResources(SettingsButton, "SettingsButton");
            SettingsButton.ImageList = serverListImageList;
            SettingsButton.Name = "SettingsButton";
            SettingsButton.UseVisualStyleBackColor = true;
            SettingsButton.Click += SettingsButton_Click;
            // 
            // gameTabControl
            // 
            resources.ApplyResources(gameTabControl, "gameTabControl");
            gameTabControl.Controls.Add(tabPage1);
            gameTabControl.Controls.Add(tabPage2);
            gameTabControl.Name = "gameTabControl";
            gameTabControl.SelectedIndex = 0;
            gameTabControl.SelectedIndexChanged += GameTabControl_SelectedIndexChanged;
            // 
            // tabPage1
            // 
            resources.ApplyResources(tabPage1, "tabPage1");
            tabPage1.Name = "tabPage1";
            tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            resources.ApplyResources(tabPage2, "tabPage2");
            tabPage2.Name = "tabPage2";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // panel3
            // 
            resources.ApplyResources(panel3, "panel3");
            panel3.BackColor = System.Drawing.SystemColors.ControlDark;
            panel3.Name = "panel3";
            // 
            // MainForm
            // 
            resources.ApplyResources(this, "$this");
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
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
            Name = "MainForm";
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

