
namespace Loader.Forms
{
    partial class ServerLoginDetailsDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ServerLoginDetailsDialog));
            usernameTextBox = new System.Windows.Forms.TextBox();
            label2 = new System.Windows.Forms.Label();
            submitButton = new System.Windows.Forms.Button();
            label1 = new System.Windows.Forms.Label();
            label3 = new System.Windows.Forms.Label();
            passwordTextBox = new System.Windows.Forms.TextBox();
            hostnameLabel = new System.Windows.Forms.LinkLabel();
            SuspendLayout();
            // 
            // usernameTextBox
            // 
            resources.ApplyResources(usernameTextBox, "usernameTextBox");
            usernameTextBox.Name = "usernameTextBox";
            usernameTextBox.ReadOnly = true;
            // 
            // label2
            // 
            resources.ApplyResources(label2, "label2");
            label2.Name = "label2";
            // 
            // submitButton
            // 
            resources.ApplyResources(submitButton, "submitButton");
            submitButton.Name = "submitButton";
            submitButton.UseVisualStyleBackColor = true;
            submitButton.Click += submitButton_Click;
            // 
            // label1
            // 
            resources.ApplyResources(label1, "label1");
            label1.Name = "label1";
            // 
            // label3
            // 
            resources.ApplyResources(label3, "label3");
            label3.Name = "label3";
            // 
            // passwordTextBox
            // 
            resources.ApplyResources(passwordTextBox, "passwordTextBox");
            passwordTextBox.Name = "passwordTextBox";
            passwordTextBox.ReadOnly = true;
            // 
            // hostnameLabel
            // 
            resources.ApplyResources(hostnameLabel, "hostnameLabel");
            hostnameLabel.Name = "hostnameLabel";
            hostnameLabel.TabStop = true;
            hostnameLabel.LinkClicked += OnLinkClicked;
            // 
            // ServerLoginDetailsDialog
            // 
            resources.ApplyResources(this, "$this");
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            Controls.Add(hostnameLabel);
            Controls.Add(label3);
            Controls.Add(passwordTextBox);
            Controls.Add(label1);
            Controls.Add(submitButton);
            Controls.Add(label2);
            Controls.Add(usernameTextBox);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "ServerLoginDetailsDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private System.Windows.Forms.TextBox usernameTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button submitButton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox passwordTextBox;
        private System.Windows.Forms.LinkLabel hostnameLabel;
    }
}