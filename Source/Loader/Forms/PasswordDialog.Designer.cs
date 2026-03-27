
namespace Loader.Forms
{
    partial class PasswordDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;
        private System.ComponentModel.ComponentResourceManager resources;
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
            resources = new System.ComponentModel.ComponentResourceManager(typeof(PasswordDialog));
            passwordTextBox = new System.Windows.Forms.TextBox();
            label2 = new System.Windows.Forms.Label();
            submitButton = new System.Windows.Forms.Button();
            SuspendLayout();
            // 
            // passwordTextBox
            // 
            resources.ApplyResources(passwordTextBox, "passwordTextBox");
            passwordTextBox.Name = "passwordTextBox";
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
            submitButton.Click += OnSubmit;
            // 
            // PasswordDialog
            // 
            resources.ApplyResources(this, "$this");
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            Controls.Add(submitButton);
            Controls.Add(label2);
            Controls.Add(passwordTextBox);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "PasswordDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            FormClosing += OnFormClosing;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private System.Windows.Forms.TextBox passwordTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button submitButton;
    }
}