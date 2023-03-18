namespace Loader
{
    partial class SettingsForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SettingsForm));
            this.UseSeperateSavesCheckbox = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.CopySavesButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // UseSeperateSavesCheckbox
            // 
            this.UseSeperateSavesCheckbox.AutoSize = true;
            this.UseSeperateSavesCheckbox.Location = new System.Drawing.Point(26, 27);
            this.UseSeperateSavesCheckbox.Name = "UseSeperateSavesCheckbox";
            this.UseSeperateSavesCheckbox.Size = new System.Drawing.Size(147, 19);
            this.UseSeperateSavesCheckbox.TabIndex = 0;
            this.UseSeperateSavesCheckbox.Text = "Use seperate save files?";
            this.UseSeperateSavesCheckbox.UseVisualStyleBackColor = true;
            this.UseSeperateSavesCheckbox.CheckedChanged += new System.EventHandler(this.SettingChanged);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(45, 49);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(448, 97);
            this.label1.TabIndex = 1;
            this.label1.Text = resources.GetString("label1.Text");
            // 
            // CopySavesButton
            // 
            this.CopySavesButton.Location = new System.Drawing.Point(320, 149);
            this.CopySavesButton.Name = "CopySavesButton";
            this.CopySavesButton.Size = new System.Drawing.Size(173, 40);
            this.CopySavesButton.TabIndex = 2;
            this.CopySavesButton.Text = "Copy Retail Saves to DS3OS";
            this.CopySavesButton.UseVisualStyleBackColor = true;
            this.CopySavesButton.Click += new System.EventHandler(this.CopySavesClicked);
            // 
            // SettingsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(528, 223);
            this.Controls.Add(this.CopySavesButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.UseSeperateSavesCheckbox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "SettingsForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Settings";
            this.Load += new System.EventHandler(this.OnLoad);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox UseSeperateSavesCheckbox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button CopySavesButton;
    }
}