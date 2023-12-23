
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
        usernameTextBox.Location = new System.Drawing.Point(108, 142);
        usernameTextBox.MaxLength = 64;
        usernameTextBox.Name = "usernameTextBox";
        usernameTextBox.ReadOnly = true;
        usernameTextBox.Size = new System.Drawing.Size(401, 23);
        usernameTextBox.TabIndex = 0;
        // 
        // label2
        // 
        label2.AutoSize = true;
        label2.Location = new System.Drawing.Point(12, 11);
        label2.Name = "label2";
        label2.Size = new System.Drawing.Size(390, 75);
        label2.TabIndex = 2;
        label2.Text = "Your server has been created and will show up in the server list shortly.\r\n\r\nYou can configure your server at the following url using the login below. \r\n\r\nPlease make sure you save the login! ";
        // 
        // submitButton
        // 
        submitButton.Location = new System.Drawing.Point(370, 222);
        submitButton.Name = "submitButton";
        submitButton.Size = new System.Drawing.Size(139, 28);
        submitButton.TabIndex = 3;
        submitButton.Text = "OK";
        submitButton.UseVisualStyleBackColor = true;
        submitButton.Click += submitButton_Click;
        // 
        // label1
        // 
        label1.AutoSize = true;
        label1.Location = new System.Drawing.Point(12, 145);
        label1.Name = "label1";
        label1.Size = new System.Drawing.Size(60, 15);
        label1.TabIndex = 4;
        label1.Text = "Username";
        // 
        // label3
        // 
        label3.AutoSize = true;
        label3.Location = new System.Drawing.Point(12, 176);
        label3.Name = "label3";
        label3.Size = new System.Drawing.Size(57, 15);
        label3.TabIndex = 6;
        label3.Text = "Password";
        // 
        // passwordTextBox
        // 
        passwordTextBox.Location = new System.Drawing.Point(108, 173);
        passwordTextBox.MaxLength = 128;
        passwordTextBox.Name = "passwordTextBox";
        passwordTextBox.ReadOnly = true;
        passwordTextBox.Size = new System.Drawing.Size(401, 23);
        passwordTextBox.TabIndex = 5;
        // 
        // hostnameLabel
        // 
        hostnameLabel.AutoSize = true;
        hostnameLabel.Location = new System.Drawing.Point(108, 116);
        hostnameLabel.Name = "hostnameLabel";
        hostnameLabel.Size = new System.Drawing.Size(60, 15);
        hostnameLabel.TabIndex = 7;
        hostnameLabel.TabStop = true;
        hostnameLabel.Text = "hostname";
        hostnameLabel.LinkClicked += OnLinkClicked;
        // 
        // ServerLoginDetailsDialog
        // 
        AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
        AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        ClientSize = new System.Drawing.Size(526, 265);
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
        StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
        Text = "Create Server";
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