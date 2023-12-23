
namespace Loader.Forms
{
    partial class CreateServerDialog
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
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CreateServerDialog));
        usernameTextBox = new System.Windows.Forms.TextBox();
        label2 = new System.Windows.Forms.Label();
        submitButton = new System.Windows.Forms.Button();
        label1 = new System.Windows.Forms.Label();
        label3 = new System.Windows.Forms.Label();
        passwordTextBox = new System.Windows.Forms.TextBox();
        label4 = new System.Windows.Forms.Label();
        label5 = new System.Windows.Forms.Label();
        SuspendLayout();
        // 
        // usernameTextBox
        // 
        usernameTextBox.Location = new System.Drawing.Point(108, 129);
        usernameTextBox.MaxLength = 64;
        usernameTextBox.Name = "usernameTextBox";
        usernameTextBox.Size = new System.Drawing.Size(522, 23);
        usernameTextBox.TabIndex = 0;
        // 
        // label2
        // 
        label2.AutoSize = true;
        label2.Location = new System.Drawing.Point(12, 11);
        label2.Name = "label2";
        label2.Size = new System.Drawing.Size(515, 90);
        label2.TabIndex = 2;
        label2.Text = resources.GetString("label2.Text");
        // 
        // submitButton
        // 
        submitButton.Location = new System.Drawing.Point(491, 246);
        submitButton.Name = "submitButton";
        submitButton.Size = new System.Drawing.Size(139, 28);
        submitButton.TabIndex = 3;
        submitButton.Text = "Create Server";
        submitButton.UseVisualStyleBackColor = true;
        submitButton.Click += OnSubmit;
        // 
        // label1
        // 
        label1.AutoSize = true;
        label1.Location = new System.Drawing.Point(12, 132);
        label1.Name = "label1";
        label1.Size = new System.Drawing.Size(74, 15);
        label1.TabIndex = 4;
        label1.Text = "Server Name";
        // 
        // label3
        // 
        label3.AutoSize = true;
        label3.Location = new System.Drawing.Point(12, 186);
        label3.Name = "label3";
        label3.Size = new System.Drawing.Size(57, 15);
        label3.TabIndex = 6;
        label3.Text = "Password";
        // 
        // passwordTextBox
        // 
        passwordTextBox.Location = new System.Drawing.Point(108, 183);
        passwordTextBox.MaxLength = 128;
        passwordTextBox.Name = "passwordTextBox";
        passwordTextBox.Size = new System.Drawing.Size(522, 23);
        passwordTextBox.TabIndex = 5;
        // 
        // label4
        // 
        label4.AutoSize = true;
        label4.Location = new System.Drawing.Point(108, 157);
        label4.Name = "label4";
        label4.Size = new System.Drawing.Size(455, 15);
        label4.TabIndex = 7;
        label4.Text = "This will be displayed in the server list to identify the server. You can change this later.";
        // 
        // label5
        // 
        label5.AutoSize = true;
        label5.Location = new System.Drawing.Point(108, 211);
        label5.Name = "label5";
        label5.Size = new System.Drawing.Size(314, 15);
        label5.TabIndex = 8;
        label5.Text = "All users will need to enter this password to join the server.";
        // 
        // CreateServerDialog
        // 
        AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
        AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        ClientSize = new System.Drawing.Size(642, 290);
        Controls.Add(label5);
        Controls.Add(label4);
        Controls.Add(label3);
        Controls.Add(passwordTextBox);
        Controls.Add(label1);
        Controls.Add(submitButton);
        Controls.Add(label2);
        Controls.Add(usernameTextBox);
        FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        Name = "CreateServerDialog";
        ShowIcon = false;
        ShowInTaskbar = false;
        StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
        Text = "Create Server";
        FormClosing += OnFormClosing;
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
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
    }
}