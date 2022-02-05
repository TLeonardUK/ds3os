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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] {
            "Name",
            "Players",
            "Description"}, 0);
            this.ImportButton = new System.Windows.Forms.Button();
            this.serverListImageList = new System.Windows.Forms.ImageList(this.components);
            this.LaunchButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.ExeLocationTextBox = new System.Windows.Forms.TextBox();
            this.ExeLocationBrowseButton = new System.Windows.Forms.Button();
            this.ImportedServerListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.BuildInfoLabel = new System.Windows.Forms.Label();
            this.ContinualUpdateTimer = new System.Windows.Forms.Timer(this.components);
            this.panel1 = new System.Windows.Forms.Panel();
            this.DiscordLink = new System.Windows.Forms.LinkLabel();
            this.GithubLink = new System.Windows.Forms.LinkLabel();
            this.RemoveButton = new System.Windows.Forms.Button();
            this.serverListRefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.RefreshButton = new System.Windows.Forms.Button();
            this.minimumPlayersBox = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.hidePasswordedBox = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.privateIpBox = new System.Windows.Forms.Label();
            this.publicIpBox = new System.Windows.Forms.Label();
            this.serverIpBox = new System.Windows.Forms.Label();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.minimumPlayersBox)).BeginInit();
            this.SuspendLayout();
            // 
            // ImportButton
            // 
            this.ImportButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.ImportButton.ImageIndex = 3;
            this.ImportButton.ImageList = this.serverListImageList;
            this.ImportButton.Location = new System.Drawing.Point(8, 425);
            this.ImportButton.Name = "ImportButton";
            this.ImportButton.Size = new System.Drawing.Size(122, 58);
            this.ImportButton.TabIndex = 1;
            this.ImportButton.Text = "Import Server";
            this.ImportButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.ImportButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
            this.ImportButton.UseVisualStyleBackColor = true;
            this.ImportButton.Click += new System.EventHandler(this.OnImportServerConfig);
            // 
            // serverListImageList
            // 
            this.serverListImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
            this.serverListImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("serverListImageList.ImageStream")));
            this.serverListImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.serverListImageList.Images.SetKeyName(0, "lock.png");
            this.serverListImageList.Images.SetKeyName(1, "arrow_refresh.png");
            this.serverListImageList.Images.SetKeyName(2, "cross.png");
            this.serverListImageList.Images.SetKeyName(3, "add.png");
            this.serverListImageList.Images.SetKeyName(4, "joystick.png");
            this.serverListImageList.Images.SetKeyName(5, "magnifier.png");
            this.serverListImageList.Images.SetKeyName(6, "bullet_star.png");
            this.serverListImageList.Images.SetKeyName(7, "star.png");
            this.serverListImageList.Images.SetKeyName(8, "world.png");
            // 
            // LaunchButton
            // 
            this.LaunchButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.LaunchButton.ImageIndex = 4;
            this.LaunchButton.ImageList = this.serverListImageList;
            this.LaunchButton.Location = new System.Drawing.Point(751, 425);
            this.LaunchButton.Name = "LaunchButton";
            this.LaunchButton.Size = new System.Drawing.Size(174, 58);
            this.LaunchButton.TabIndex = 2;
            this.LaunchButton.Text = "Launch Game";
            this.LaunchButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.LaunchButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
            this.LaunchButton.UseVisualStyleBackColor = true;
            this.LaunchButton.Click += new System.EventHandler(this.OnLaunch);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
            this.label2.Location = new System.Drawing.Point(8, 157);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(138, 15);
            this.label2.TabIndex = 4;
            this.label2.Text = "DarkSoulsIII.exe Location";
            // 
            // ExeLocationTextBox
            // 
            this.ExeLocationTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ExeLocationTextBox.Location = new System.Drawing.Point(152, 153);
            this.ExeLocationTextBox.Name = "ExeLocationTextBox";
            this.ExeLocationTextBox.ReadOnly = true;
            this.ExeLocationTextBox.Size = new System.Drawing.Size(724, 23);
            this.ExeLocationTextBox.TabIndex = 5;
            // 
            // ExeLocationBrowseButton
            // 
            this.ExeLocationBrowseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ExeLocationBrowseButton.ImageIndex = 5;
            this.ExeLocationBrowseButton.ImageList = this.serverListImageList;
            this.ExeLocationBrowseButton.Location = new System.Drawing.Point(883, 153);
            this.ExeLocationBrowseButton.Name = "ExeLocationBrowseButton";
            this.ExeLocationBrowseButton.Size = new System.Drawing.Size(42, 23);
            this.ExeLocationBrowseButton.TabIndex = 6;
            this.ExeLocationBrowseButton.UseVisualStyleBackColor = true;
            this.ExeLocationBrowseButton.Click += new System.EventHandler(this.OnBrowseForExecutable);
            // 
            // ImportedServerListView
            // 
            this.ImportedServerListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ImportedServerListView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ImportedServerListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader4,
            this.columnHeader2});
            this.ImportedServerListView.FullRowSelect = true;
            this.ImportedServerListView.GridLines = true;
            this.ImportedServerListView.HideSelection = false;
            listViewItem1.StateImageIndex = 0;
            this.ImportedServerListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1});
            this.ImportedServerListView.Location = new System.Drawing.Point(8, 190);
            this.ImportedServerListView.MultiSelect = false;
            this.ImportedServerListView.Name = "ImportedServerListView";
            this.ImportedServerListView.Size = new System.Drawing.Size(917, 229);
            this.ImportedServerListView.SmallImageList = this.serverListImageList;
            this.ImportedServerListView.Sorting = System.Windows.Forms.SortOrder.Descending;
            this.ImportedServerListView.TabIndex = 7;
            this.ImportedServerListView.UseCompatibleStateImageBehavior = false;
            this.ImportedServerListView.View = System.Windows.Forms.View.Details;
            this.ImportedServerListView.SelectedIndexChanged += new System.EventHandler(this.OnSelectedServerChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Server Name";
            this.columnHeader1.Width = 250;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Player Count";
            this.columnHeader4.Width = 100;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Description";
            this.columnHeader2.Width = 500;
            // 
            // BuildInfoLabel
            // 
            this.BuildInfoLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.BuildInfoLabel.Location = new System.Drawing.Point(152, 130);
            this.BuildInfoLabel.Name = "BuildInfoLabel";
            this.BuildInfoLabel.Size = new System.Drawing.Size(724, 20);
            this.BuildInfoLabel.TabIndex = 8;
            this.BuildInfoLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // ContinualUpdateTimer
            // 
            this.ContinualUpdateTimer.Interval = 5000;
            this.ContinualUpdateTimer.Tick += new System.EventHandler(this.OnContinualUpdateTimer);
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.BackColor = System.Drawing.Color.Black;
            this.panel1.BackgroundImage = global::Loader.Properties.Resources.banner2;
            this.panel1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel1.Controls.Add(this.DiscordLink);
            this.panel1.Controls.Add(this.GithubLink);
            this.panel1.Location = new System.Drawing.Point(-2, -2);
            this.panel1.Margin = new System.Windows.Forms.Padding(0);
            this.panel1.Name = "panel1";
            this.panel1.Padding = new System.Windows.Forms.Padding(5);
            this.panel1.Size = new System.Drawing.Size(943, 127);
            this.panel1.TabIndex = 10;
            // 
            // DiscordLink
            // 
            this.DiscordLink.ActiveLinkColor = System.Drawing.Color.White;
            this.DiscordLink.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.DiscordLink.AutoSize = true;
            this.DiscordLink.BackColor = System.Drawing.Color.Transparent;
            this.DiscordLink.Cursor = System.Windows.Forms.Cursors.Hand;
            this.DiscordLink.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.DiscordLink.ForeColor = System.Drawing.Color.White;
            this.DiscordLink.LinkColor = System.Drawing.Color.White;
            this.DiscordLink.Location = new System.Drawing.Point(164, 83);
            this.DiscordLink.Name = "DiscordLink";
            this.DiscordLink.Size = new System.Drawing.Size(95, 21);
            this.DiscordLink.TabIndex = 13;
            this.DiscordLink.TabStop = true;
            this.DiscordLink.Text = "Join Discord";
            this.DiscordLink.VisitedLinkColor = System.Drawing.Color.Gray;
            this.DiscordLink.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.ClickDiscordLink);
            // 
            // GithubLink
            // 
            this.GithubLink.ActiveLinkColor = System.Drawing.Color.White;
            this.GithubLink.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.GithubLink.AutoSize = true;
            this.GithubLink.BackColor = System.Drawing.Color.Transparent;
            this.GithubLink.Cursor = System.Windows.Forms.Cursors.Hand;
            this.GithubLink.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.GithubLink.ForeColor = System.Drawing.Color.White;
            this.GithubLink.LinkColor = System.Drawing.Color.White;
            this.GithubLink.Location = new System.Drawing.Point(65, 83);
            this.GithubLink.Name = "GithubLink";
            this.GithubLink.Size = new System.Drawing.Size(93, 21);
            this.GithubLink.TabIndex = 12;
            this.GithubLink.TabStop = true;
            this.GithubLink.Text = "Visit GitHub";
            this.GithubLink.VisitedLinkColor = System.Drawing.Color.Gray;
            this.GithubLink.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.ClickGithubLink);
            // 
            // RemoveButton
            // 
            this.RemoveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.RemoveButton.ImageIndex = 2;
            this.RemoveButton.ImageList = this.serverListImageList;
            this.RemoveButton.Location = new System.Drawing.Point(136, 425);
            this.RemoveButton.Name = "RemoveButton";
            this.RemoveButton.Size = new System.Drawing.Size(37, 58);
            this.RemoveButton.TabIndex = 9;
            this.RemoveButton.UseVisualStyleBackColor = true;
            this.RemoveButton.Click += new System.EventHandler(this.OnRemoveClicked);
            // 
            // serverListRefreshTimer
            // 
            this.serverListRefreshTimer.Enabled = true;
            this.serverListRefreshTimer.Interval = 30000;
            this.serverListRefreshTimer.Tick += new System.EventHandler(this.OnServerRefreshTimer);
            // 
            // RefreshButton
            // 
            this.RefreshButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.RefreshButton.ImageIndex = 1;
            this.RefreshButton.ImageList = this.serverListImageList;
            this.RefreshButton.Location = new System.Drawing.Point(180, 425);
            this.RefreshButton.Name = "RefreshButton";
            this.RefreshButton.Size = new System.Drawing.Size(37, 58);
            this.RefreshButton.TabIndex = 11;
            this.RefreshButton.UseVisualStyleBackColor = true;
            this.RefreshButton.Click += new System.EventHandler(this.OnRefreshClicked);
            // 
            // minimumPlayersBox
            // 
            this.minimumPlayersBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.minimumPlayersBox.Location = new System.Drawing.Point(325, 428);
            this.minimumPlayersBox.Name = "minimumPlayersBox";
            this.minimumPlayersBox.Size = new System.Drawing.Size(53, 23);
            this.minimumPlayersBox.TabIndex = 12;
            this.minimumPlayersBox.ValueChanged += new System.EventHandler(this.FilterPropertyChanged);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(223, 431);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(100, 15);
            this.label1.TabIndex = 13;
            this.label1.Text = "Minimum Players";
            // 
            // hidePasswordedBox
            // 
            this.hidePasswordedBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.hidePasswordedBox.AutoSize = true;
            this.hidePasswordedBox.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.hidePasswordedBox.Location = new System.Drawing.Point(223, 458);
            this.hidePasswordedBox.Name = "hidePasswordedBox";
            this.hidePasswordedBox.Size = new System.Drawing.Size(117, 19);
            this.hidePasswordedBox.TabIndex = 14;
            this.hidePasswordedBox.Text = "Hide Passworded";
            this.hidePasswordedBox.UseVisualStyleBackColor = true;
            this.hidePasswordedBox.CheckedChanged += new System.EventHandler(this.FilterPropertyChanged);
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label3.Location = new System.Drawing.Point(542, 427);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(100, 23);
            this.label3.TabIndex = 15;
            this.label3.Text = "Server IP";
            this.label3.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.Location = new System.Drawing.Point(542, 446);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(102, 15);
            this.label4.TabIndex = 17;
            this.label4.Text = "Private IP";
            this.label4.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.Location = new System.Drawing.Point(542, 465);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(102, 15);
            this.label5.TabIndex = 19;
            this.label5.Text = "Public IP";
            this.label5.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // privateIpBox
            // 
            this.privateIpBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.privateIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.privateIpBox.Location = new System.Drawing.Point(650, 446);
            this.privateIpBox.Name = "privateIpBox";
            this.privateIpBox.Size = new System.Drawing.Size(99, 19);
            this.privateIpBox.TabIndex = 21;
            this.privateIpBox.Text = "255.255.255.255";
            // 
            // publicIpBox
            // 
            this.publicIpBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.publicIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.publicIpBox.Location = new System.Drawing.Point(650, 465);
            this.publicIpBox.Name = "publicIpBox";
            this.publicIpBox.Size = new System.Drawing.Size(99, 19);
            this.publicIpBox.TabIndex = 22;
            this.publicIpBox.Text = "255.255.255.255";
            // 
            // serverIpBox
            // 
            this.serverIpBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.serverIpBox.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.serverIpBox.Location = new System.Drawing.Point(650, 427);
            this.serverIpBox.Name = "serverIpBox";
            this.serverIpBox.Size = new System.Drawing.Size(99, 19);
            this.serverIpBox.TabIndex = 23;
            this.serverIpBox.Text = "255.255.255.255";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(934, 487);
            this.Controls.Add(this.serverIpBox);
            this.Controls.Add(this.publicIpBox);
            this.Controls.Add(this.privateIpBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.hidePasswordedBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.minimumPlayersBox);
            this.Controls.Add(this.RefreshButton);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.RemoveButton);
            this.Controls.Add(this.BuildInfoLabel);
            this.Controls.Add(this.ImportedServerListView);
            this.Controls.Add(this.ExeLocationBrowseButton);
            this.Controls.Add(this.ExeLocationTextBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.LaunchButton);
            this.Controls.Add(this.ImportButton);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(950, 490);
            this.Name = "MainForm";
            this.Text = "Dark Souls III - Open Server Loader";
            this.Load += new System.EventHandler(this.OnLoaded);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.minimumPlayersBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button ImportButton;
        private System.Windows.Forms.Button LaunchButton;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox ExeLocationTextBox;
        private System.Windows.Forms.Button ExeLocationBrowseButton;
        private System.Windows.Forms.ListView ImportedServerListView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Label BuildInfoLabel;
        private System.Windows.Forms.Timer ContinualUpdateTimer;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.Button RemoveButton;
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
    }
}

