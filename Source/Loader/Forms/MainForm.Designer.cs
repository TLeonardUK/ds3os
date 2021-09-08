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
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] {
            "Name",
            "Players",
            "Description"}, 0);
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.ImportButton = new System.Windows.Forms.Button();
            this.LaunchButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.ExeLocationTextBox = new System.Windows.Forms.TextBox();
            this.ExeLocationBrowseButton = new System.Windows.Forms.Button();
            this.ImportedServerListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.serverListImageList = new System.Windows.Forms.ImageList(this.components);
            this.BuildInfoLabel = new System.Windows.Forms.Label();
            this.ContinualUpdateTimer = new System.Windows.Forms.Timer(this.components);
            this.panel1 = new System.Windows.Forms.Panel();
            this.RemoveButton = new System.Windows.Forms.Button();
            this.serverListRefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // ImportButton
            // 
            this.ImportButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.ImportButton.Location = new System.Drawing.Point(8, 404);
            this.ImportButton.Name = "ImportButton";
            this.ImportButton.Size = new System.Drawing.Size(111, 37);
            this.ImportButton.TabIndex = 1;
            this.ImportButton.Text = "Import Server";
            this.ImportButton.UseVisualStyleBackColor = true;
            this.ImportButton.Click += new System.EventHandler(this.OnImportServerConfig);
            // 
            // LaunchButton
            // 
            this.LaunchButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.LaunchButton.Location = new System.Drawing.Point(677, 404);
            this.LaunchButton.Name = "LaunchButton";
            this.LaunchButton.Size = new System.Drawing.Size(248, 37);
            this.LaunchButton.TabIndex = 2;
            this.LaunchButton.Text = "Launch Game";
            this.LaunchButton.UseVisualStyleBackColor = true;
            this.LaunchButton.Click += new System.EventHandler(this.OnLaunch);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
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
            this.ExeLocationBrowseButton.Location = new System.Drawing.Point(883, 153);
            this.ExeLocationBrowseButton.Name = "ExeLocationBrowseButton";
            this.ExeLocationBrowseButton.Size = new System.Drawing.Size(42, 23);
            this.ExeLocationBrowseButton.TabIndex = 6;
            this.ExeLocationBrowseButton.Text = "...";
            this.ExeLocationBrowseButton.UseVisualStyleBackColor = true;
            this.ExeLocationBrowseButton.Click += new System.EventHandler(this.OnBrowseForExecutable);
            // 
            // ImportedServerListView
            // 
            this.ImportedServerListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ImportedServerListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader4,
            this.columnHeader2});
            this.ImportedServerListView.FullRowSelect = true;
            this.ImportedServerListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.ImportedServerListView.HideSelection = false;
            this.ImportedServerListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1});
            this.ImportedServerListView.Location = new System.Drawing.Point(8, 190);
            this.ImportedServerListView.MultiSelect = false;
            this.ImportedServerListView.Name = "ImportedServerListView";
            this.ImportedServerListView.Size = new System.Drawing.Size(917, 200);
            this.ImportedServerListView.SmallImageList = this.serverListImageList;
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
            // serverListImageList
            // 
            this.serverListImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
            this.serverListImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("serverListImageList.ImageStream")));
            this.serverListImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.serverListImageList.Images.SetKeyName(0, "lock.png");
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
            this.ContinualUpdateTimer.Interval = 1000;
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
            this.panel1.Location = new System.Drawing.Point(-2, -2);
            this.panel1.Margin = new System.Windows.Forms.Padding(0);
            this.panel1.Name = "panel1";
            this.panel1.Padding = new System.Windows.Forms.Padding(5);
            this.panel1.Size = new System.Drawing.Size(943, 127);
            this.panel1.TabIndex = 10;
            // 
            // RemoveButton
            // 
            this.RemoveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.RemoveButton.Location = new System.Drawing.Point(125, 404);
            this.RemoveButton.Name = "RemoveButton";
            this.RemoveButton.Size = new System.Drawing.Size(111, 37);
            this.RemoveButton.TabIndex = 9;
            this.RemoveButton.Text = "Remove Server";
            this.RemoveButton.UseVisualStyleBackColor = true;
            this.RemoveButton.Click += new System.EventHandler(this.OnRemoveClicked);
            // 
            // serverListRefreshTimer
            // 
            this.serverListRefreshTimer.Enabled = true;
            this.serverListRefreshTimer.Interval = 30000;
            this.serverListRefreshTimer.Tick += new System.EventHandler(this.OnServerRefreshTimer);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(934, 451);
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
    }
}

