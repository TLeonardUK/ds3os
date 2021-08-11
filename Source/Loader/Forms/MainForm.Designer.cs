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
            this.ImportButton = new System.Windows.Forms.Button();
            this.LaunchButton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.ExeLocationTextBox = new System.Windows.Forms.TextBox();
            this.ExeLocationBrowseButton = new System.Windows.Forms.Button();
            this.ImportedServerListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.BuildInfoLabel = new System.Windows.Forms.Label();
            this.RemoveButton = new System.Windows.Forms.Button();
            this.RunningProcessUpdateTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // ImportButton
            // 
            this.ImportButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.ImportButton.Location = new System.Drawing.Point(8, 280);
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
            this.LaunchButton.Location = new System.Drawing.Point(653, 280);
            this.LaunchButton.Name = "LaunchButton";
            this.LaunchButton.Size = new System.Drawing.Size(143, 37);
            this.LaunchButton.TabIndex = 2;
            this.LaunchButton.Text = "Launch Game";
            this.LaunchButton.UseVisualStyleBackColor = true;
            this.LaunchButton.Click += new System.EventHandler(this.OnLaunch);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 70);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(96, 15);
            this.label1.TabIndex = 3;
            this.label1.Text = "Imported Servers";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(8, 14);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(138, 15);
            this.label2.TabIndex = 4;
            this.label2.Text = "DarkSoulsIII.exe Location";
            // 
            // ExeLocationTextBox
            // 
            this.ExeLocationTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ExeLocationTextBox.Location = new System.Drawing.Point(8, 37);
            this.ExeLocationTextBox.Name = "ExeLocationTextBox";
            this.ExeLocationTextBox.ReadOnly = true;
            this.ExeLocationTextBox.Size = new System.Drawing.Size(739, 23);
            this.ExeLocationTextBox.TabIndex = 5;
            // 
            // ExeLocationBrowseButton
            // 
            this.ExeLocationBrowseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ExeLocationBrowseButton.Location = new System.Drawing.Point(754, 37);
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
            this.columnHeader3,
            this.columnHeader2});
            this.ImportedServerListView.FullRowSelect = true;
            this.ImportedServerListView.HideSelection = false;
            this.ImportedServerListView.Location = new System.Drawing.Point(8, 92);
            this.ImportedServerListView.Name = "ImportedServerListView";
            this.ImportedServerListView.Size = new System.Drawing.Size(788, 180);
            this.ImportedServerListView.TabIndex = 7;
            this.ImportedServerListView.UseCompatibleStateImageBehavior = false;
            this.ImportedServerListView.View = System.Windows.Forms.View.Details;
            this.ImportedServerListView.SelectedIndexChanged += new System.EventHandler(this.OnSelectedServerChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Server Name";
            this.columnHeader1.Width = 150;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Hostname";
            this.columnHeader3.Width = 150;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Description";
            this.columnHeader2.Width = 460;
            // 
            // BuildInfoLabel
            // 
            this.BuildInfoLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.BuildInfoLabel.Location = new System.Drawing.Point(152, 14);
            this.BuildInfoLabel.Name = "BuildInfoLabel";
            this.BuildInfoLabel.Size = new System.Drawing.Size(595, 20);
            this.BuildInfoLabel.TabIndex = 8;
            this.BuildInfoLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // RemoveButton
            // 
            this.RemoveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.RemoveButton.Location = new System.Drawing.Point(125, 280);
            this.RemoveButton.Name = "RemoveButton";
            this.RemoveButton.Size = new System.Drawing.Size(111, 37);
            this.RemoveButton.TabIndex = 9;
            this.RemoveButton.Text = "Remove Server";
            this.RemoveButton.UseVisualStyleBackColor = true;
            this.RemoveButton.Click += new System.EventHandler(this.OnRemoveClicked);
            // 
            // RunningProcessUpdateTimer
            // 
            this.RunningProcessUpdateTimer.Interval = 1000;
            this.RunningProcessUpdateTimer.Tick += new System.EventHandler(this.OnProcessUpdateTimer);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(805, 327);
            this.Controls.Add(this.RemoveButton);
            this.Controls.Add(this.BuildInfoLabel);
            this.Controls.Add(this.ImportedServerListView);
            this.Controls.Add(this.ExeLocationBrowseButton);
            this.Controls.Add(this.ExeLocationTextBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.LaunchButton);
            this.Controls.Add(this.ImportButton);
            this.Name = "MainForm";
            this.Text = "Dark Souls 3 - Open Server Loader";
            this.Load += new System.EventHandler(this.OnLoaded);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button ImportButton;
        private System.Windows.Forms.Button LaunchButton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox ExeLocationTextBox;
        private System.Windows.Forms.Button ExeLocationBrowseButton;
        private System.Windows.Forms.ListView ImportedServerListView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Label BuildInfoLabel;
        private System.Windows.Forms.Button RemoveButton;
        private System.Windows.Forms.Timer RunningProcessUpdateTimer;
    }
}

