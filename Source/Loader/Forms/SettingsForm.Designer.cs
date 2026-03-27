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
            UseSeperateSavesCheckbox = new System.Windows.Forms.CheckBox();
            label1 = new System.Windows.Forms.Label();
            CopySavesButton = new System.Windows.Forms.Button();
            SuspendLayout();
            // 
            // UseSeperateSavesCheckbox
            // 
            resources.ApplyResources(UseSeperateSavesCheckbox, "UseSeperateSavesCheckbox");
            UseSeperateSavesCheckbox.Name = "UseSeperateSavesCheckbox";
            UseSeperateSavesCheckbox.UseVisualStyleBackColor = true;
            UseSeperateSavesCheckbox.CheckedChanged += SettingChanged;
            // 
            // label1
            // 
            resources.ApplyResources(label1, "label1");
            label1.Name = "label1";
            // 
            // CopySavesButton
            // 
            resources.ApplyResources(CopySavesButton, "CopySavesButton");
            CopySavesButton.Name = "CopySavesButton";
            CopySavesButton.UseVisualStyleBackColor = true;
            CopySavesButton.Click += CopySavesClicked;
            // 
            // SettingsForm
            // 
            resources.ApplyResources(this, "$this");
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            Controls.Add(CopySavesButton);
            Controls.Add(label1);
            Controls.Add(UseSeperateSavesCheckbox);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            Name = "SettingsForm";
            ShowInTaskbar = false;
            Load += OnLoad;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private System.Windows.Forms.CheckBox UseSeperateSavesCheckbox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button CopySavesButton;
    }
}